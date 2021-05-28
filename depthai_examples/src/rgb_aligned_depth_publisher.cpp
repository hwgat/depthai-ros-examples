
#include "ros/ros.h"

#include <iostream>
#include <cstdio>
#include "sensor_msgs/Image.h"
#include "stereo_msgs/DisparityImage.h"
#include <camera_info_manager/camera_info_manager.h>
#include <depthai_examples/depth_align.hpp>
#include <functional>

// Inludes common necessary includes for development using depthai library
#include "depthai/depthai.hpp"
#include <depthai_bridge/BridgePublisher.hpp>
#include <depthai_bridge/ImageConverter.hpp>
#include <depthai_bridge/DisparityConverter.hpp>

int main(int argc, char** argv){

    ros::init(argc, argv, "rgb_depth_aligned");
    ros::NodeHandle pnh("~");
    
    std::string deviceName, mode;
    std::string camera_param_uri;
    int bad_params = 0;
    
    bad_params += !pnh.getParam("camera_name", deviceName);
    bad_params += !pnh.getParam("camera_param_uri", camera_param_uri);
    bad_params += !pnh.getParam("mode", mode);

    if (bad_params > 0)
    {
        throw std::runtime_error("Couldn't find one of the parameters");
    }

    RgbStereoExampe stero_pipeline;
    if(mode == "depth"){
        stero_pipeline.initDepthaiDev(true);
    }
    else{
        stero_pipeline.initDepthaiDev(false);
    }
    std::vector<std::shared_ptr<dai::DataOutputQueue>> imageDataQueues = stero_pipeline.getExposedImageStreams();

    // this part would be removed once we have calibration-api
    std::string rgb_uri = camera_param_uri +"/" + "color_resized.yaml";
    std::string stereo_uri = camera_param_uri + "/" + "color_resized.yaml";
   
    dai::rosBridge::ImageConverter rgbConverter(deviceName + "_rgb_camera_optical_frame", true);
    dai::rosBridge::BridgePublisher<sensor_msgs::Image, dai::ImgFrame> rgbPublish(imageDataQueues[0],
                                                                                    pnh, 
                                                                                    std::string("color/image"),
                                                                                    std::bind(&dai::rosBridge::ImageConverter::toRosMsg, 
                                                                                    &rgbConverter, 
                                                                                    std::placeholders::_1, 
                                                                                    std::placeholders::_2) , 
                                                                                    30,
                                                                                    rgb_uri,
                                                                                    "color");

    rgbPublish.addPubisherCallback();


     if(mode == "depth"){
        dai::rosBridge::ImageConverter depthConverter(deviceName + "_right_camera_optical_frame", true);
        dai::rosBridge::BridgePublisher<sensor_msgs::Image, dai::ImgFrame> depthPublish(imageDataQueues[1],
                                                                                     pnh, 
                                                                                     std::string("stereo/depth"),
                                                                                     std::bind(&dai::rosBridge::ImageConverter::toRosMsg, 
                                                                                     &depthConverter, // since the converter has the same frame name
                                                                                                      // and image type is also same we can reuse it
                                                                                     std::placeholders::_1, 
                                                                                     std::placeholders::_2) , 
                                                                                     30,
                                                                                     stereo_uri,
                                                                                     "stereo");
        depthPublish.addPubisherCallback();
        ros::spin();
    }
   else{
        dai::rosBridge::DisparityConverter dispConverter(deviceName + "_right_camera_optical_frame", 880, 7.5, 20, 2000);
        dai::rosBridge::BridgePublisher<stereo_msgs::DisparityImage, dai::ImgFrame> dispPublish(imageDataQueues[1],
                                                                                     pnh, 
                                                                                     std::string("stereo/disparity"),
                                                                                     std::bind(&dai::rosBridge::DisparityConverter::toRosMsg, 
                                                                                     &dispConverter, 
                                                                                     std::placeholders::_1, 
                                                                                     std::placeholders::_2) , 
                                                                                     30,
                                                                                     stereo_uri,
                                                                                     "stereo");
        dispPublish.addPubisherCallback();
        ros::spin();
    }

    // We can add the rectified frames also similar to these publishers. 
    // Left them out so that users can play with it by adding and removing

    
    return 0;
}
