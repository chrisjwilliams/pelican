#ifndef ABSTRACTLOCKABLEDATA_H
#define ABSTRACTLOCKABLEDATA_H


#include "AbstractLockable.h"

/**
 * @file AbstractLockableData.h
 */

namespace pelican {
    class Data;

/**
 * @class AbstractLockableData
 *  
 * @brief
 *    Interface for data classes that can be locked
 * @details
 * 
 */

class AbstractLockableData : public AbstractLockable
{
    Q_OBJECT

    public:
        AbstractLockableData( QObject* parent );
        virtual ~AbstractLockableData();

        /// return the data id 
        QString id() const;

        /// set the object identity
        void setId(const QString&);

        /// return the underlying data object
        Data* data() const { return _data; } ;

    protected:
        Data* _data;
};

} // namespace pelican
#endif // ABSTRACTLOCKABLEDATA_H 