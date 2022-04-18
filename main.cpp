
#include <iostream>
#include <cstdio>

#include <Common/Base/hkBase.h>
#include <Common/Base/Memory/System/Util/hkMemoryInitUtil.h>
#include <Common/Base/System/Io/IStream/hkIStream.h>
#include <Common/Base/Reflection/Registry/hkDefaultClassNameRegistry.h>
#include <Common/Serialize/Util/hkRootLevelContainer.h>
#include <Common/Serialize/Util/hkLoader.h>
#include <Common/Serialize/Util/hkSerializeUtil.h>
#include <Common/Serialize/Version/hkVersionPatchManager.h>
#include <Common/Base/Reflection/hkInternalClassMember.h>
#include <Common/Serialize/Util/hkSerializeDeprecated.h>

#include <Animation/Animation/hkaAnimationContainer.h>
#include <shellapi.h>
#include <locale>
#include <codecvt>

#include "Common/Base/System/Init/PlatformInit.cxx"

static void HK_CALL errorReport(const char* msg, void* userContext)
{
    using namespace std;
    printf("%s", msg);
}

void init() {
    PlatformInit();
    hkMemoryRouter* memoryRouter = hkMemoryInitUtil::initDefault(hkMallocAllocator::m_defaultMallocAllocator, hkMemorySystem::FrameInfo(1024 * 1024));
    hkBaseSystem::init(memoryRouter, errorReport);
    PlatformFileSystemInit();
    hkSerializeDeprecatedInit::initDeprecated();
}

inline std::string convert_from_wstring(const std::wstring &wstr)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv;
    return conv.to_bytes(wstr);
}

// animassist.exe mode base_hkx base_hkx_anim_index new_hkx hex_hkx_anim_index output
int main(int argc, const char** argv) {

    int nargc = 0;
    wchar_t** nargv;

    auto command_line = GetCommandLineW();
    if (command_line == nullptr)
    {
        printf("Fatal error.");
        return 1;
    }
    nargv = CommandLineToArgvW(command_line, &nargc);
    if (nargv == nullptr)
    {
        printf("Fatal error.");
        return 1;
    }

    int mode;

    hkStringBuf anim_hkt_1;
    int anim_index_1;

    hkStringBuf anim_hkt_2;
    int anim_index_2;

    hkStringBuf out;
    hkRootLevelContainer* anim_root_container_1;
    hkRootLevelContainer* anim_root_container_2;

    mode = _wtoi(nargv[1]);
    anim_hkt_1 = convert_from_wstring(nargv[2]).c_str();
    anim_index_1 = _wtoi(nargv[3]);
    anim_hkt_2 = convert_from_wstring(nargv[4]).c_str();
    anim_index_2 = _wtoi(nargv[5]);
    out = convert_from_wstring(nargv[6]).c_str();

    init();
    auto loader = new hkLoader();

    anim_root_container_1 = loader->load(anim_hkt_1);
    anim_root_container_2 = loader->load(anim_hkt_2);

    hkOstream stream(out);
    hkPackfileWriter::Options packOptions;

    auto anim_container_1 = reinterpret_cast<hkaAnimationContainer*>(anim_root_container_1->findObjectByType(hkaAnimationContainerClass.getName()));
    auto anim_container_2 = reinterpret_cast<hkaAnimationContainer*>(anim_root_container_2->findObjectByType(hkaAnimationContainerClass.getName()));

    if (mode == 1) { // replace
        auto anim_ptr_2 = anim_container_2->m_animations[anim_index_2];
        auto binding_ptr_2 = anim_container_2->m_bindings[anim_index_2];

        anim_container_1->m_animations[anim_index_1] = anim_ptr_2; // replace hkx_1 animation with that of hkx_2
        anim_container_1->m_bindings[anim_index_1] = binding_ptr_2;
    }
    else if (mode == 2) { // remove
        anim_container_1->m_animations.removeAt(anim_index_1);
        anim_container_1->m_bindings.removeAt(anim_index_1);
    }
    else if (mode == 3) { // add
        auto anim_ptr_2 = anim_container_2->m_animations[anim_index_2];
        auto binding_ptr_2 = anim_container_2->m_bindings[anim_index_2];

        anim_container_1->m_animations.pushBack(anim_ptr_2);
        anim_container_1->m_bindings.pushBack(binding_ptr_2);
    }

    hkResult res = hkSerializeUtil::saveTagfile(anim_root_container_1, hkRootLevelContainer::staticClass(), stream.getStreamWriter(), nullptr, hkSerializeUtil::SAVE_DEFAULT);

    if (res.isSuccess()) {
        // I had some cleanup here. And then Havok decided to access violate every time.
        return 0;
    } else {
        std::cout << "\n\nAn error occurred while saving the HKX...\n";
        return 1;
    }
}

#include <Common/Base/keycode.cxx>

#undef HK_FEATURE_PRODUCT_AI
//#undef HK_FEATURE_PRODUCT_ANIMATION
#undef HK_FEATURE_PRODUCT_CLOTH
#undef HK_FEATURE_PRODUCT_DESTRUCTION_2012
#undef HK_FEATURE_PRODUCT_DESTRUCTION
#undef HK_FEATURE_PRODUCT_BEHAVIOR
#undef HK_FEATURE_PRODUCT_PHYSICS_2012
#undef HK_FEATURE_PRODUCT_SIMULATION
#undef HK_FEATURE_PRODUCT_PHYSICS

#define HK_SERIALIZE_MIN_COMPATIBLE_VERSION 201130r1

#include <Common/Base/Config/hkProductFeatures.cxx>