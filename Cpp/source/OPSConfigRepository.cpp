/**
* 
* Copyright (C) 2016-2020 Lennart Andersson.
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

#include "Lockable.h"
#include "Participant.h" 
#include "BasicError.h"
#include "Domain.h"
#include "DefaultOPSConfigImpl.h"
#include "ConfigException.h"
#include "TimeHelper.h"

#include "OPSConfigRepository.h"

namespace ops {

// A recursive mutex (same thread can take it several times)
static Lockable repoLock;


OPSConfigRepository* OPSConfigRepository::Instance()
{
    SafeLock lock(&repoLock);
    static OPSConfigRepository repo;
    return &repo;
}

OPSConfigRepository::OPSConfigRepository() :
    m_config(new OPSConfig())
{
}

OPSConfigRepository::~OPSConfigRepository()
{
    // Since we just borrow the references to domains (they are owned by file cache)
    // we must clear the domain list in our OPSConfig object
    Clear();
}

bool OPSConfigRepository::domainExist(ObjectName_T domainID )
{
    SafeLock lock(&repoLock);
    std::vector<Domain*>& doms = m_config->getRefToDomains();
    for (unsigned int i = 0; i < doms.size(); i++) {
        if (doms[i]->getDomainID() == domainID) return true;
    }
    return false;
}

int OPSConfigRepository::numDomains()
{
	return (int)m_config->getRefToDomains().size();
}

// Add domains from OPS configuration file "filename"
// if 'domain' == "", all domains will be added otherwise only the specified 'domain'
// Returns true if at least one domain added
bool OPSConfigRepository::Add( FileName_T filename, ObjectName_T domain )
{
    bool retVal = false;

    SafeLock lock(&repoLock);

    if (domain != "") {
        // Check if domain already exist
        if (domainExist( domain )) {
			ErrorMessage_T msg("domain '");
			msg += domain;
			msg += "' already exist";
    		BasicError err("OPSConfigRepository", "Add", msg );
            Participant::reportStaticError(&err);
            return retVal;
        }
    }

    std::shared_ptr<OPSConfig> config = nullptr;

    try {
        // Check if file already read
        if (m_configFiles.find(filename) != m_configFiles.end()) {
            config = m_configFiles[filename];

        } else {
            // Need to read file
            config = OPSConfig::getConfig( filename );
			if (config == nullptr) { return retVal; }
            m_configFiles[filename] = config;
        }
    } catch (ops::ConfigException& ex)
    {
		ErrorMessage_T msg("Exception: ");
		msg += ex.what();
        BasicError err("OPSConfigRepository", "Add", msg);
        Participant::reportStaticError(&err);
        return retVal;
    }

	return extractDomains(config, domain);
}

// Add all domains from the given OPSConfig object. To be used in cases were the config
// is constructed in some other way than directly from a file.
// Note that the repository takes over ownership of the config object
bool OPSConfigRepository::Add(std::shared_ptr<OPSConfig> config)
{
	bool retVal = false;

	// Construct a new unique name to be used in the "file cache"
	FileName_T name("Internal_");
	name += NumberToString((size_t)config.get());

	SafeLock lock(&repoLock);

	// Check if name already used
	if (m_configFiles.find(name) != m_configFiles.end()) {
		ErrorMessage_T msg("Failed to add OPSConfig to repository");
		BasicError err("OPSConfigRepository", "Add", msg);
		Participant::reportStaticError(&err);
		return retVal;

	} else {
		m_configFiles[name] = config;
	}

	return extractDomains(config);
}

bool OPSConfigRepository::extractDomains(std::shared_ptr<OPSConfig>& config, ObjectName_T domain)
{
	bool retVal = false;

	// Get all domains read from file 
	std::vector<ops::Domain* > domains = config->getDomains();
	// Add the choosen one(s) to our list if not already there
	for (unsigned int i = 0; i < domains.size(); i++) {
		if ((domain == "") || (domains[i]->getDomainID() == domain)) {
			if (domainExist(domains[i]->getDomainID())) {
				ErrorMessage_T msg("domain '");
				msg += domains[i]->getDomainID();
				msg += "' already exist";
				BasicError err("OPSConfigRepository", "Add", msg);
				Participant::reportStaticError(&err);
			} else {
				// Add unique domains to our list
				m_config->getRefToDomains().push_back(domains[i]);
				retVal = true;
			}
		}
	}

	return retVal;
}

void OPSConfigRepository::Clear()
{
    SafeLock lock(&repoLock);
    // Since we just borrow the references to domains (they are owned by file cache)
    // we can just clear the domain list in our OPSConfig object
    m_config->getRefToDomains().clear();
}

// Remove all domains from repository and clears the file-cache
void OPSConfigRepository::TotalClear()
{
	SafeLock lock(&repoLock);
	m_config->getRefToDomains().clear();

	for (auto it = m_configFiles.begin(); it != m_configFiles.end(); ++it) {
		it->second.reset();
	}
	m_configFiles.clear();
}

// Get a reference to the OPSConfig object
// if 'domainID' != "", the domain must exist otherwise nullptr is returned.
std::shared_ptr<OPSConfig> OPSConfigRepository::getConfig(ObjectName_T domainID )
{
    SafeLock lock(&repoLock);

    // If no domain have been added, we try to add the default file
    // This is for backward compatibility
    if (m_config->getRefToDomains().size() == 0) {
        if (!Add("ops_config.xml")) return nullptr;
    }

    if (domainID != "") {
        if (!domainExist( domainID )) return nullptr;
    }

    return m_config;
}

}
