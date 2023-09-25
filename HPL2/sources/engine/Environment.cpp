#include <engine/Environment.h>

namespace hpl {
    namespace Environment {
        EnvironmentStore& GetStore() {
            static EnvironmentStore instance;
            return instance;
        }
    }


    EnvironmentStore::EnvironmentStore() {

    }
    EnvironmentStore::~EnvironmentStore() {

    }

    //EnvironmentResult AddVariable(uint32_t id, void* variable) {
    //}

    EnvironmentResult EnvironmentStore::AddOrAllocateVariable(uint32_t id, size_t size) {
        EnvironmentResult result;
        auto itemIt = m_variableMap.find(id);
        if(itemIt != m_variableMap.end()) {
            result.m_variable = itemIt->second;
            result.m_status = EnvironmentResult::ResultStatus::Found;
            return result;
        }
        void* data = malloc(size);
        if(data == nullptr) {
            result.m_status = EnvironmentResult::ResultStatus::OutOfMemory;
            return result;
        }
        m_variableMap.insert({id, data});
        result.m_variable = data;
        result.m_status = EnvironmentResult::ResultStatus::Created;
        return result;
    }

    EnvironmentResult EnvironmentStore::GetVariable(uint32_t id) {
        EnvironmentResult result;
        auto itemIt = m_variableMap.find(id);
        if(itemIt != m_variableMap.end()) {
            result.m_variable = itemIt->second;
            result.m_status = EnvironmentResult::ResultStatus::Found;
            return result;
        }
        result.m_status = EnvironmentResult::ResultStatus::NotFound;
        return result;
    }

    EnvironmentResult EnvironmentStore::RemoveVariable(uint32_t id) {
        EnvironmentResult result;
        auto itemIt = m_variableMap.find(id);
        if(itemIt != m_variableMap.end()) {
            result.m_variable = itemIt->second;
            result.m_status = EnvironmentResult::ResultStatus::Found;
            m_variableMap.erase(itemIt);
            return result;
        }
        result.m_status = EnvironmentResult::ResultStatus::NotFound;
        return result;
    }

    EnvironmentResult EnvironmentStore::RemoveAndDeallocateVariable(uint32_t id) {
        EnvironmentResult result = RemoveVariable(id);
        if(result.m_status == EnvironmentResult::ResultStatus::Found) {
            free(result.m_variable);
            result.m_variable = nullptr;
        }
        return result;
    }


}
