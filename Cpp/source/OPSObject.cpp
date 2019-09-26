/**
* 
* Copyright (C) 2006-2009 Anton Gravestam.
* Copyright (C) 2019 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include "OPSObject.h"

namespace ops
{

#if defined(DEBUG_OPSOBJECT_COUNTER)
    std::atomic<uint32_t> OPSObject::_NumOpsObjects = 0;
#endif

    OPSObject::OPSObject()
    {
        key = "k";
        typesString = "";
#if defined(DEBUG_OPSOBJECT_COUNTER)
        _NumOpsObjects++;
#endif
    }

	// Copy constructor
    OPSObject::OPSObject(const OPSObject& other)
    {
        key = other.key;
        typesString = other.typesString;
        spareBytes = other.spareBytes;
#if defined(DEBUG_OPSOBJECT_COUNTER)
        _NumOpsObjects++;
#endif
    }

	// Move constructor
	OPSObject::OPSObject(OPSObject&& other)
	{
		// Take other's resources
		key = other.key;
		typesString = other.typesString;
		spareBytes = other.spareBytes;
#if defined(DEBUG_OPSOBJECT_COUNTER)
		_NumOpsObjects++;
#endif
	}

	// Copy assignment operator
    OPSObject& OPSObject::operator= (OPSObject other)
    {
		if (this == &other) { return *this; }
        key = other.key;
        typesString = other.typesString;
        spareBytes = other.spareBytes;
        return *this;
    }

	OPSObject* OPSObject::clone()
	{
		OPSObject* obj = new OPSObject();
		fillClone(obj);
		return obj;
	}

	void OPSObject::fillClone(OPSObject* obj) const
	{
		obj->key = key;
		obj->typesString = typesString;
		// Copy spareBytes vector
		size_t len = spareBytes.size();
		obj->spareBytes.reserve(len);
		obj->spareBytes.resize(len, 0);
		if (len > 0) { memcpy((void*)&obj->spareBytes[0], (void*)&spareBytes[0], len); }
	}

	ObjectKey_T OPSObject::getKey()
    {
         return key;
    }

	const TypeId_T& OPSObject::getTypeString()
	{
		return typesString;
	}

	void OPSObject::setKey(ObjectKey_T k)
	{
		key = k;
	}

	void OPSObject::serialize(ArchiverInOut* archive)
	{
		archive->inout("key", key);
	}
	
    OPSObject::~OPSObject()
    {
#if defined(DEBUG_OPSOBJECT_COUNTER)
        _NumOpsObjects--;
#endif
    }
}
