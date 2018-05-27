mkdir build
cd build

cmake -DCMAKE_BUILD_TYPE=RELEASE -DVK_INCLUDE_DIR="/d/Coding/Vulkan/1.0.68.0/Include" \
	-DVK_LIBRARY="/d/Coding/C++/vulkanengine/vulkan-1.dll" \
	-DGLM_INCLUDE_DIR="/d/Coding/C++/Libraries/glm" \
-G "MinGW Makefiles" ..
