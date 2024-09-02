#pragma once

#include "Resources/XMLFile.h"
#include "Resources/MeshVertices.h"
#include "Rendering/Shader.h"
#include "Rendering/Material.h"
#include "Rendering/Texture.h"
#include "Physics/PhysicsMaterial.h"
#include "Font.h"
#include "Timer.h"

namespace CgEngine {

    template<typename R>
    class ResourceMap {
    public:
        using Iterator = typename std::unordered_map<std::string, std::unique_ptr<R>>::iterator;

    public:
        explicit ResourceMap(bool canResLoadAsync) : canResLoadAsync(canResLoadAsync) {};

        R* get(const std::string& name) const {
            return map.at(name).get();
        }

        void insert(const std::string& name, R* resource) {
            map.insert({name, std::unique_ptr<R>(resource)});
        }

        bool contains(const std::string& name) const {
            return map.find(name) != map.end();
        }

        bool canResourceLoadAsync() const {
            return canResLoadAsync;
        }

        Iterator begin() {
            return map.begin();
        }

        Iterator end() {
            return map.end();
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<R>> map{};
        bool canResLoadAsync;
    };

    class ResourceManager {
    public:
        ResourceManager() {
            registerResourceType<XMLFile>();
            registerResourceType<MeshVertices>();
            registerResourceType<Shader>();
            registerResourceType<ComputeShader>();
            registerResourceType<Material>();
            registerResourceType<Texture2D>();
            registerResourceType<TextureCube>();
            registerResourceType<PhysicsMaterial>();
            registerResourceType<Font>();
        }

        template<typename R>
        R* getResource(const std::string& name) {
            auto& resourceMap = getResourceMap<R>();
            if (resourceMap.contains(name)) {
                return resourceMap.get(name);
            }
            R* resource = R::createResource(name);

            if (R::canLoadAsync) {
                resource->resourceManagerLoadAsync();
            }

            resourceMap.insert(name, resource);

            return resource;
        }

        template<typename R, typename S>
        R* getResource(const std::string& name, const S& spec) {
            auto& resourceMap = getResourceMap<R>();
            if (resourceMap.contains(name)) {
                return resourceMap.get(name);
            }
            R* resource = R::createResource(name, spec);

            if (R::canLoadAsync) {
                resource->resourceManagerLoadAsync();
            }

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
            resourceMap.insert(name, resource);
            return true;
        }

        void updateAsyncResources() {
            for (const auto& [typeName, rm]: resourceMaps) {
                const auto& resourceMap = static_cast<ResourceMap<Resource>*>(rm);

                if (!resourceMap->canResourceLoadAsync()) {
                    continue;
                }

                for (const auto& [rn, resource]: *resourceMap) {
                    if (!resource->isLoaded() && resource->resourceManagerAsyncLoadingFinished()) {
                        resource->resourceManagerSetAsyncLoadedData();
                    }
                }
            }
        }

    private:
        std::unordered_map<const char*, void*> resourceMaps;

        template<typename R>
        void registerResourceType() {
            const char* typeName = typeid(R).name();
            resourceMaps.insert({typeName, new ResourceMap<R>(R::canLoadAsync)});
        }

        template<typename R>
        ResourceMap<R>& getResourceMap() {
            const char* typeName = typeid(R).name();
            return *static_cast<ResourceMap<R>*>(resourceMaps.at(typeName));
        }
    };

}
