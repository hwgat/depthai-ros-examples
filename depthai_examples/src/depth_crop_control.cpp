/**
 * This example shows usage of depth camera in crop mode with the possibility to move the crop.
 * Use 'WASD' in order to do it.
 */
#include "ros/ros.h"

#include <iostream>
#include <memory>

#include <depthai_ros_msgs/NormalizedImageCrop.h>
#include "sensor_msgs/Image.h"

#include <depthai_bridge/BridgePublisher.hpp>
#include <depthai_bridge/ImageConverter.hpp>
#include "depthai/depthai.hpp"


// Step size ('W','A','S','D' controls)
static constexpr float stepSize = 0.02;
std::shared_ptr<dai::DataInputQueue> configQueue;

bool cropDepthImage(depthai_ros_msgs::NormalizedImageCrop::Request request, depthai_ros_msgs::NormalizedImageCrop::Response response){
    dai::ImageManipConfig cfg;
    cfg.setCropRect(request.topLeft.x, request.topLeft.y, request.bottomRight.x, request.bottomRight.y);
    configQueue->send(cfg);
    return true;    
}

int main() {

    ros::init(argc, argv, "depth_crop_control");
    ros::NodeHandle pnh("~");
    std::string cameraName;

    int badParams = 0;
    badParams += !pnh.getParam("camera_name", cameraName);

    if (badParams > 0)
    {   
        std::cout << " Bad parameters -> " << badParams << std::endl;
        throw std::runtime_error("Couldn't find %d of the parameters");
    }

    ros::ServiceServer service = n.advertiseService("crop_control_srv", cropDepthImage);

    // Create pipeline
    dai::Pipeline pipeline;

    // Define sources and outputs
    auto monoRight = pipeline.create<dai::node::MonoCamera>();
    auto monoLeft = pipeline.create<dai::node::MonoCamera>();
    auto manip = pipeline.create<dai::node::ImageManip>();
    auto stereo = pipeline.create<dai::node::StereoDepth>();

    auto configIn = pipeline.create<dai::node::XLinkIn>();
    auto xout = pipeline.create<dai::node::XLinkOut>();

    configIn->setStreamName("config");
    xout->setStreamName("depth");

    // Crop range
    dai::Point2f topLeft(0.2, 0.2);
    dai::Point2f bottomRight(0.8, 0.8);

    // Properties
    monoRight->setBoardSocket(dai::CameraBoardSocket::RIGHT);
    monoLeft->setBoardSocket(dai::CameraBoardSocket::LEFT);
    monoRight->setResolution(dai::MonoCameraProperties::SensorResolution::THE_400_P);
    monoLeft->setResolution(dai::MonoCameraProperties::SensorResolution::THE_400_P);

    manip->initialConfig.setCropRect(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
    manip->setMaxOutputFrameSize(monoRight->getResolutionHeight() * monoRight->getResolutionWidth() * 3);
    // stereo->initialConfig.setConfidenceThreshold(200);

    // Linking
    configIn->out.link(manip->inputConfig);
    stereo->depth.link(manip->inputImage);
    manip->out.link(xout->input);
    monoRight->out.link(stereo->left);
    monoLeft->out.link(stereo->right);

    // Connect to device and start pipeline
    dai::Device device(pipeline);

    // Queues
    auto depthQueue = device.getOutputQueue(xout->getStreamName(), 5, false);
    configQueue = device.getInputQueue(configIn->getStreamName());

    auto calibrationHandler = device.readCalibration();

    dai::rosBridge::ImageConverter depthConverter(cameraName + "_right_camera_optical_frame", true);
    auto rightCameraInfo = converter.calibrationToCameraInfo(calibrationHandler, dai::CameraBoardSocket::RIGHT, 1280, 720); 

    dai::rosBridge::BridgePublisher<sensor_msgs::Image, dai::ImgFrame> depthPublish(depthQueue,
                                                                                    pnh, 
                                                                                    std::string("stereo/depth"),
                                                                                    std::bind(&dai::rosBridge::ImageConverter::toRosMsg, 
                                                                                    &depthConverter, // since the converter has the same frame name
                                                                                                    // and image type is also same we can reuse it
                                                                                    std::placeholders::_1, 
                                                                                    std::placeholders::_2) , 
                                                                                    30,
                                                                                    rightCameraInfo,
                                                                                    "stereo");
    depthPublish.addPubisherCallback();
    ros::spin();

    return 0;
}
