#include "pelican/core/DataTypes.h"
#include "pelican/core/AbstractStreamAdapter.h"
#include "pelican/core/AbstractServiceAdapter.h"
#include "pelican/core/DataClientFactory.h"
#include "pelican/data/DataSpec.h"
#include <iostream>


namespace pelican {


DataTypes::DataTypes()
    : _adapterFactory(0)
{
}

DataTypes::DataTypes(  const ConfigNode& config, AbstractAdapterFactory* factory )
    : _conf(config), _adapterFactory(factory)
{
    _adapterNames = _conf.getOptionHash("data", "type", "adapter");
}

/**
 *@details
 */
DataTypes::~DataTypes()
{
}

/**
 * @details
 * adds a data requirement. If the appropriate adapter
 * has not already been added with setAdater() then
 * this method will attemp to create a suitable one.
 */
void DataTypes::addData( const DataSpec& data )
{
    _dataRequirements.append(data);
    const QHash<QString,QString>& defaultAdapters = data.getAdapterTypes();
    _addData( data.serviceData(), defaultAdapters, AbstractAdapter::Service );
    _addData( data.streamData(), defaultAdapters, AbstractAdapter::Stream );
}

void DataTypes::_addData( const QSet<QString>& data, const QHash<QString,QString>& defaultAdapters,
         const AbstractAdapter::AdapterType_t& defaultType ) 
{
    foreach( const QString& dataType, data ) {
        if( ! adapterAvailable( dataType ) ) {
            if( ! _adapterNames.contains(dataType) 
                && defaultAdapters.contains(dataType)
                && defaultAdapters.value(dataType) != "" ) {
                // Use the default value if there is one
                // and the config does not override it
                setAdapter(dataType, _createAdapter(defaultAdapters.value(dataType),
                           defaultType ) );
            } else {
                // use the configuration specified adapter
                // if available
                // will throw if nothing is found
                setAdapter(dataType, _createAdapter(dataType, defaultType ) );
            }
        }
        else {
            _setAdapter( _dataRequirements.last(), dataType );
        }
    }
}

void DataTypes::addData(const QList<DataSpec>& data )
{
    foreach( const DataSpec& s, data) {
        addData(s);
    }
}

/**
 * @details
 * Sets a particular adapter for a particular type of data. All data added
 * with addData will be made consistent with the adpater
 */
void DataTypes::setAdapter(const QString& type, AbstractAdapter* adapter)
{
    _adapters[type] = adapter;
    // ensure each data requirement is of the correct type
    for(int i=0; i < _dataRequirements.size(); ++i )
    {
        DataSpec& req = _dataRequirements[i];
        _setAdapter( req, type );
    }
}

void DataTypes::_setAdapter( DataSpec& req, const QString& type )
{
    switch( _adapters[type]->type() )
    {
        case AbstractAdapter::Service :
            req.setServiceData( type );
            break;
        case AbstractAdapter::Stream :
            req.setStreamData( type );
            break;
        default:
            throw QString("unknown adapter type");
            break;
    }
}

AbstractStreamAdapter* DataTypes::streamAdapter(const QString& type) const
{
    Q_ASSERT( _adapters.contains(type) );
    Q_ASSERT( _adapters.value(type)->type() == AbstractAdapter::Stream );
    return static_cast<AbstractStreamAdapter*>(_adapters.value(type));
}

AbstractServiceAdapter* DataTypes::serviceAdapter(const QString& type) const
{
    Q_ASSERT( _adapters.contains(type) );
    Q_ASSERT( _adapters.value(type)->type() == AbstractAdapter::Service );
    return static_cast<AbstractServiceAdapter*>(_adapters.value(type));
}

AbstractAdapter::AdapterType_t DataTypes::type(const QString& dataName) const
{
    Q_ASSERT( _adapters.contains(dataName) );
    return _adapters.value(dataName)->type();
}

const QList<DataSpec>& DataTypes::dataSpec() const
{
    return _dataRequirements;
}

bool DataTypes::adapterAvailable( const QString& type ) const {
     return _adapters.contains(type);
}

AbstractAdapter* DataTypes::_createAdapter( const QString& dataType,
                const AbstractAdapter::AdapterType_t& defaultType ) const {

    // Find the configuration information for adapters.
    QString adapterType = _adapterNames.value(dataType);
    if( adapterType == "" ) throw QString("No adpater type specified for stream \"%1\"").arg(dataType);
    AbstractAdapter* a = _adapterFactory->create( adapterType ,
                _conf.getNamedOption("data","name","") );
    if( a->type() == AbstractAdapter::Unknown ) {
        a->setType( defaultType );
    }
    return a;
}

} // namespace pelican
