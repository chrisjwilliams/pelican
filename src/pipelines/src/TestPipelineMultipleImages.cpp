#include "pipelines/TestPipelineMultipleImages.h"
#include "modules/ZenithImagerDft.h"
#include "modules/ImageWriterFits.h"
#include "modules/ZenithModelVisibilities.h"
#include <iostream>

#include "utility/memCheck.h"

namespace pelican {


/**
 * @details
 */
TestPipelineMultipleImages::TestPipelineMultipleImages()
    : AbstractPipeline()
{
}


/**
 * @details
 */
TestPipelineMultipleImages::~TestPipelineMultipleImages()
{
}


/**
 * @details
 */
void TestPipelineMultipleImages::init()
{
    setName("TestPipelineDirtyImage");
    _visModel = static_cast<ZenithModelVisibilities*>(createModule("ZenithModelVisibilities"));

    _imagerA = static_cast<ZenithImagerDft*>(createModule("ZenithImagerDft", "a"));
    _imagerB = static_cast<ZenithImagerDft*>(createModule("ZenithImagerDft", "b"));

    _fitsWriterA = static_cast<ImageWriterFits*>(createModule("ImageWriterFits", "a"));
    _fitsWriterB = static_cast<ImageWriterFits*>(createModule("ImageWriterFits", "b"));
}


/**
 * @details
 */
void TestPipelineMultipleImages::run(QHash<QString, DataBlob*>& data)
{
    _visModel->run(data);
    std::cout << "model done\n";

    _imagerA->run(data);
    _fitsWriterA->run(data);
    std::cout << "A done\n";

    _imagerB->run(data);
    _fitsWriterB->run(data);
    std::cout << "B done\n";

    stop();
}


} // namespace pelican
