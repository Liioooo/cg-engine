#pragma once

namespace CgEngine {

    class Resource {
    public:
        virtual ~Resource() = default;

        virtual void resourceManagerLoadAsync() {};
        virtual bool resourceManagerAsyncLoadingFinished() { return true; }
        virtual void resourceManagerSetAsyncLoadedData() {};

        static inline bool canLoadAsync = false;

        bool isLoaded() const;

    protected:
        void setLoaded();

    private:
        bool loaded = false;
    };

}
