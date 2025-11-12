//
// Created by Akashic on 7/1/2025.
//

#ifndef TYPEDEF_HPP
#define TYPEDEF_HPP
#include <cstdint>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#include <string>
#include <windows.h>

extern "C" IMAGE_DOS_HEADER __ImageBase;

namespace fi {
    using RVA = uint32_t;

    IMAGE_NT_HEADERS *image_header_from_base();

    using ResolveAddressPtr = uintptr_t (*)(uint32_t ordinal);
    struct FakeImportConfig {
        std::string mock_dll_name{};
        ResolveAddressPtr resolve_address{nullptr};
        static const FakeImportConfig& config();
        static void set_config(FakeImportConfig&& config);

    };

    void load_all_imports();
}

#endif //TYPEDEF_HPP
