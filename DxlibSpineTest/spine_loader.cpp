

#include "spine_loader.h"

std::shared_ptr<spine::SkeletonData> spine_loader::ReadTextSkeletonFromFile(const spine::String& filePath, spine::Atlas* atlas, float scale)
{
	spine::SkeletonJson json(atlas);
	json.setScale(scale);
	auto skeletonData = json.readSkeletonDataFile(filePath);
	if (!skeletonData)
	{
		return nullptr;
	}
	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}

std::shared_ptr<spine::SkeletonData> spine_loader::ReadBinarySkeletonFromFile(const spine::String& filePath, spine::Atlas* atlas, float scale)
{
	spine::SkeletonBinary binary(atlas);
	binary.setScale(scale);
	auto skeletonData = binary.readSkeletonDataFile(filePath);
	if (!skeletonData)
	{
		return nullptr;
	}
	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}

std::shared_ptr<spine::SkeletonData> spine_loader::ReadTextSkeletonFromMemory(const std::string& skeletonJson, spine::Atlas* atlas, float scale)
{
	spine::SkeletonJson json(atlas);
	json.setScale(scale);
	auto skeletonData = json.readSkeletonData(skeletonJson.c_str());
	if (!skeletonData)
	{
		return nullptr;
	}
	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}

std::shared_ptr<spine::SkeletonData> spine_loader::ReadBinarySkeletonFromMemory(const std::string& skeletonBinary, spine::Atlas* atlas, float scale)
{
	spine::SkeletonBinary binary(atlas);
	binary.setScale(scale);
	auto skeletonData = binary.readSkeletonData(reinterpret_cast<const unsigned char*>(skeletonBinary.c_str()), static_cast<int>(skeletonBinary.size()));
	if (!skeletonData)
	{
		return nullptr;
	}
	return std::shared_ptr<spine::SkeletonData>(skeletonData);
}
