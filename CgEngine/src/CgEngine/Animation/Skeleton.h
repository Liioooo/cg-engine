#pragma once

namespace CgEngine {

    class Skeleton {
    public:
        static const uint32_t NoBone = ~0;

        Skeleton(uint32_t boneCount);

        uint32_t addBone(const std::string& name, uint32_t parent, const glm::mat4& transform);
        void setArmatureTransform(uint32_t index, const glm::mat4& transform);
        uint32_t findBoneIndex(const std::string_view name) const;
        uint32_t getParentBoneIndex(uint32_t index) const;
        const std::string& getBoneName(uint32_t index) const;
        const std::vector<glm::mat4>& getBoneTransforms() const;
        const glm::mat4& getArmatureTransform(uint32_t index) const;

        void calculateMissingArmatureTransforms();

        uint32_t getNumBones() const;

    private:
        std::vector<std::string> boneNames;
        std::vector<uint32_t> parentBoneIndices;
        std::vector<glm::mat4> boneTransforms;
        std::unordered_map<uint32_t, glm::mat4> armatureTransforms;
    };

}
