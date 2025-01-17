#pragma once

#include "Rendering/CustomShaders.h"
#include "Resources/MeshVertices.h"
#include "Resources/AudioFile.h"
#include "Rendering/PBRMaterial.h"
#include "Rendering/Texture.h"
#include "Physics/PhysicsMaterial.h"
#include "Font.h"
#include "Timer.h"
#include "ResRef.h"

namespace CgEngine {

    template<typename R>
    class ResourceMap {
    public:
        using Iterator = typename std::unordered_map<std::string, ResRef<R>>::iterator;

    public:
        explicit ResourceMap() {};

        ResRef<R> get(const std::string& name) const {
            return map.at(name);
        }

        void insert(const std::string& name, ResRef<R> resource) {
            map.insert({name, resource});
        }

        bool contains(const std::string& name) const {
            return map.find(name) != map.end();
        }

        Iterator begin() {
            return map.begin();
        }

        Iterator end() {
            return map.end();
        }

        void erase(const Iterator& it) {
            map.erase(it);
        }

    private:
        std::unordered_map<std::string, ResRef<R>> map{};
    };

    class ResourceManager {
    public:
        ResourceManager() {
            registerResourceType<MeshVertices>();
            registerResourceType<PBRMaterial>();
            registerResourceType<Texture2D>();
            registerResourceType<TextureCube>();
            registerResourceType<PhysicsMaterial>();
            registerResourceType<Font>();
            registerResourceType<CustomShader>();
            registerResourceType<CustomComputeShader>();
            registerResourceType<AudioFile>();
        }

        template<typename R>
        ResRef<R> getResource(const std::string& name) {
            auto& resourceMap = getResourceMap<R>();
            if (resourceMap.contains(name)) {
                return resourceMap.get(name);
            }
            ResRef<R> resource = ResRef<R>(R::createResource(name));
            resourceMap.insert(name, resource);
            return resource;
        }

        template<typename R, typename S>
        ResRef<R> getResource(const std::string& name, const S& spec) {
            auto& resourceMap = getResourceMap<R>();
            if (resourceMap.contains(name)) {
                return resourceMap.get(name);
            }
            ResRef<R> resource = ResRef<R>(R::createResource(name, spec));
            resourceMap.insert(name, resource);
            return resource;
        }

        template<typename R>
        bool hasResource(const std::string& name) {
            auto& resourceMap = getResourceMap<R>();
            return resourceMap.contains(name);
        }

        template<typename R>
        bool insertResource(const std::string& name, R* resource) {
            auto& resourceMap = getResourceMap<R>();
            if (resourceMap.contains(name)) {
                return false;
            }
            resourceMap.insert(name, ResRef<R>(resource));
            return true;
        }

        template<typename R>
        typename ResourceMap<R>::Iterator begin() {
            return getResourceMap<R>().begin();
        }

        template<typename R>
        typename ResourceMap<R>::Iterator end() {
            return getResourceMap<R>().end();
        }

        void unloadUnusedResources() {
            unloadUnusedResourceType<MeshVertices>();
            unloadUnusedResourceType<PBRMaterial>();
            unloadUnusedResourceType<Texture2D>();
            unloadUnusedResourceType<TextureCube>();
            unloadUnusedResourceType<PhysicsMaterial>();
            unloadUnusedResourceType<Font>();
            unloadUnusedResourceType<CustomShader>();
            unloadUnusedResourceType<CustomComputeShader>();
            unloadUnusedResourceType<AudioFile>();
        }

    private:
        std::unordered_map<const char*, void*> resourceMaps;

        template<typename R>
        void registerResourceType() {
            const char* typeName = typeid(R).name();
            resourceMaps.insert({typeName, new ResourceMap<R>()});
        }

        template<typename R>
        ResourceMap<R>& getResourceMap() {
            const char* typeName = typeid(R).name();
            return *static_cast<ResourceMap<R>*>(resourceMaps.at(typeName));
        }

        template<typename R>
        void unloadUnusedResourceType() {
            ResourceMap<R>& resourceMap = getResourceMap<R>();
            for (auto it = resourceMap.begin(); it != resourceMap.end();) {
                CG_LOGGING_DEBUG("Unload Resource Info: {0} : {1} : UseCount: {2}", typeid(R).name(), it->first, it->second.use_count())

                if (it->second.use_count() == 1) {
                    CG_LOGGING_DEBUG("Unloading Resource: {0} : {1}", typeid(R).name(), it->first)
                    resourceMap.erase(it++);
                } else {
                    ++it;
                }
            }
        }
    };

}
