#ifndef SPINE_LOADER_H_
#define SPINE_LOADER_H_

#include <string>
#include <memory>

#include <spine/spine.h>

namespace spine_loader
{
	std::shared_ptr<spine::SkeletonData> ReadTextSkeletonFromFile(const spine::String& filePath, spine::Atlas* atlas, float scale = 1.f);
	std::shared_ptr<spine::SkeletonData> ReadBinarySkeletonFromFile(const spine::String& filePath, spine::Atlas* atlas, float scale = 1.f);

	std::shared_ptr<spine::SkeletonData> ReadTextSkeletonFromMemory(const std::string& skeletonJson, spine::Atlas* atlas, float scale = 1.f);
	std::shared_ptr<spine::SkeletonData> ReadBinarySkeletonFromMemory(const std::string& skeletonBinary, spine::Atlas* atlas, float scale = 1.f);
}

#endif
