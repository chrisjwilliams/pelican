#include "adapters/AbstractAdapter.h"

#include "utility/memCheck.h"

namespace pelican {

/**
 * @details
 * Creates a new abstract Pelican adapter with the given configuration.
 */
AbstractAdapter::AbstractAdapter(const ConfigNode& config)
{
    _config = &config;
}


/**
 * @details
 * Destroys the adapter.
 */
AbstractAdapter::~AbstractAdapter()
{
}


} // namespace pelican
