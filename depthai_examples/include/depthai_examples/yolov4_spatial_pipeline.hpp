#pragma once

#include <vector>
#include "depthai/depthai.hpp"

class YoloSpatialDetectionExample{

    public:

    static const std::vector<std::string> label_map ;

    YoloSpatialDetectionExample() = default;
    ~YoloSpatialDetectionExample() = default;

    void initDepthaiDev(std::string nnPath);
    void initDepthaiDev(std::string nnPath, std::string mxID);

    std::vector<std::shared_ptr<dai::DataOutputQueue>> getExposedImageStreams();
    std::vector<std::shared_ptr<dai::DataOutputQueue>> getExposedNnetStreams();
    
    private:
    std::vector<std::shared_ptr<dai::DataOutputQueue>> _opImageStreams;
    std::vector<std::shared_ptr<dai::DataOutputQueue>> _opNNetStreams;

    std::unique_ptr<dai::Device> _dev;
    dai::DeviceInfo _dev_info;

    dai::Pipeline _p;

};