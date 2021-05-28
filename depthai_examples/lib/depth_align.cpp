#include <depthai_examples/depth_align.hpp>


void RgbStereoExampe::initDepthaiDev(bool withDepth){
    
    auto camRgb = _p.create<dai::node::ColorCamera>();
    auto left = _p.create<dai::node::MonoCamera>();
    auto right = _p.create<dai::node::MonoCamera>();
    auto stereo = _p.create<dai::node::StereoDepth>();

    auto rgbOut = _p.create<dai::node::XLinkOut>();
    auto depthOut = _p.create<dai::node::XLinkOut>();

    rgbOut->setStreamName("rgb");
    if (withDepth) {
        depthOut->setStreamName("depth");
    }
    else {
        depthOut->setStreamName("disparity");
    }

    camRgb->setBoardSocket(dai::CameraBoardSocket::RGB);
    camRgb->setResolution(dai::ColorCameraProperties::SensorResolution::THE_1080_P);
    camRgb->setIspScale(2, 3);
    // For now, RGB needs fixed focus to properly align with depth.
    // This value was used during calibration
    camRgb->initialControl.setManualFocus(135);

    left->setResolution(dai::MonoCameraProperties::SensorResolution::THE_720_P);
    left->setBoardSocket(dai::CameraBoardSocket::LEFT);
    right->setResolution(dai::MonoCameraProperties::SensorResolution::THE_720_P);
    right->setBoardSocket(dai::CameraBoardSocket::RIGHT);

    stereo->setConfidenceThreshold(200);
    // LR-check is required for depth alignment
    stereo->setLeftRightCheck(true);
    stereo->setDepthAlign(dai::CameraBoardSocket::RGB);

    camRgb->isp.link(rgbOut->input);
    left->out.link(stereo->left);
    right->out.link(stereo->right);
    if(withDepth){
        stereo->depth.link(depthOut->input);
    }
    else{
        stereo->disparity.link(depthOut->input);
    }

    // CONNECT TO DEVICE
     _dev = std::make_unique<dai::Device>(_p);
    //  _dev->startPipeline();

    _opImageStreams.push_back(_dev->getOutputQueue("rgb", 30, false));
    if (withDepth) {
        _opImageStreams.push_back(_dev->getOutputQueue("depth", 30, false));
    }else{
        _opImageStreams.push_back(_dev->getOutputQueue("disparity", 30, false));
    }
}

std::vector<std::shared_ptr<dai::DataOutputQueue>> RgbStereoExampe::getExposedImageStreams(){
        return _opImageStreams;
}
