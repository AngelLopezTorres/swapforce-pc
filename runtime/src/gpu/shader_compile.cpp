// Compilacion HLSL -> DXIL (DXC) con cache en disco por hash y prueba H2 (--shader-test).
#include <windows.h>
#include <unknwn.h>
#include <objidl.h>
#include <cstdarg>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "dxc/dxcapi.h"
#include "ucode_hlsl.h"
#include "ucode_info.h"

namespace rt::gpu {

namespace {

IDxcCompiler3* g_compiler = nullptr;
std::string g_dxcError;

bool EnsureDxc() {
    if (g_compiler) return true;
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    std::string dir = path;
    dir = dir.substr(0, dir.find_last_of("\\/") + 1);
    HMODULE m = LoadLibraryExA((dir + "dxcompiler.dll").c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!m) { g_dxcError = "no se puede cargar dxcompiler.dll junto al ejecutable (" + dir + ")"; return false; }
    auto create = (DxcCreateInstanceProc)GetProcAddress(m, "DxcCreateInstance");
    if (!create) { g_dxcError = "DxcCreateInstance no encontrado"; return false; }
    // CLSID_DxcCompiler = 73e22d93-e6ce-47f3-b5bf-f0664f39c1b0 (dxcapi.h)
    static const GUID clsid = {0x73e22d93, 0xe6ce, 0x47f3, {0xb5, 0xbf, 0xf0, 0x66, 0x4f, 0x39, 0xc1, 0xb0}};
    HRESULT hr = create(clsid, __uuidof(IDxcCompiler3), (void**)&g_compiler);
    if (FAILED(hr) || !g_compiler) { g_dxcError = "DxcCreateInstance(IDxcCompiler3) fallo hr=" + std::to_string((unsigned)hr); return false; }
    return true;
}

std::wstring W(const char* s) { std::wstring w; while (*s) w += (wchar_t)*s++; return w; }

} // namespace

bool CompileHlslToDxil(const std::string& hlsl, bool vertex, std::string& dxil, std::string& err) {
    return CompileHlslProfile(hlsl, vertex ? L"vs_6_0" : L"ps_6_0", dxil, err);
}

bool CompileHlslProfile(const std::string& hlsl, const wchar_t* profile, std::string& dxil, std::string& err) {
    if (!EnsureDxc()) { err = g_dxcError; return false; }
    DxcBuffer src = {hlsl.data(), hlsl.size(), 65001 /* CP_UTF8 */};
    std::vector<LPCWSTR> args = {L"-T", profile, L"-E", L"main", L"-HV", L"2021", L"-Qstrip_debug", L"-Qstrip_reflect"};
    IDxcResult* res = nullptr;
    HRESULT hr = g_compiler->Compile(&src, args.data(), (UINT32)args.size(), nullptr, __uuidof(IDxcResult), (void**)&res);
    if (FAILED(hr) || !res) { err = "IDxcCompiler3::Compile hr=" + std::to_string((unsigned)hr); return false; }
    HRESULT status = S_OK;
    res->GetStatus(&status);
    IDxcBlobUtf8* errors = nullptr;
    res->GetOutput(DXC_OUT_ERRORS, __uuidof(IDxcBlobUtf8), (void**)&errors, nullptr);
    if (errors && errors->GetStringLength()) err.assign(errors->GetStringPointer(), errors->GetStringLength());
    if (errors) errors->Release();
    bool ok = false;
    if (SUCCEEDED(status)) {
        IDxcBlob* obj = nullptr;
        res->GetOutput(DXC_OUT_OBJECT, __uuidof(IDxcBlob), (void**)&obj, nullptr);
        if (obj && obj->GetBufferSize()) { dxil.assign((const char*)obj->GetBufferPointer(), obj->GetBufferSize()); ok = true; }
        if (obj) obj->Release();
    }
    res->Release();
    return ok;
}

std::string RectListGeometryShaderHlsl() {
    std::string o;
    o += "struct VSOut\n{\n    float4 pos : SV_Position;\n";
    for (int i = 0; i < 16; ++i) { o += "    float4 i" + std::to_string(i) + " : TEXCOORD" + std::to_string(i) + ";\n"; }
    o += "};\n";
    o += "VSOut xe_comb(VSOut a, VSOut b, VSOut c)\n{\n    VSOut r;\n    r.pos = b.pos + c.pos - a.pos;\n";
    for (int i = 0; i < 16; ++i) { std::string n = "i" + std::to_string(i); o += "    r." + n + " = b." + n + " + c." + n + " - a." + n + ";\n"; }
    o += "    return r;\n}\n";
    o += "[maxvertexcount(4)]\nvoid main(triangle VSOut v[3], inout TriangleStream<VSOut> s)\n{\n";
    o += "    float2 e12 = v[2].pos.xy - v[1].pos.xy, e20 = v[0].pos.xy - v[2].pos.xy, e01 = v[1].pos.xy - v[0].pos.xy;\n";
    o += "    float l12 = dot(e12, e12), l20 = dot(e20, e20), l01 = dot(e01, e01);\n";
    o += "    uint ia, ib, ic;\n";
    o += "    if (l12 > l20 && l12 > l01) { ia = 0; ib = 1; ic = 2; }\n";
    o += "    else if (l20 > l01) { ia = 1; ib = 2; ic = 0; }\n";
    o += "    else { ia = 2; ib = 0; ic = 1; }\n";
    o += "    VSOut d = xe_comb(v[ia], v[ib], v[ic]);\n";
    o += "    s.Append(v[ia]); s.Append(v[ib]); s.Append(v[ic]); s.Append(d);\n    s.RestartStrip();\n}\n";
    return o;
}

namespace {
bool ReadFile(const std::string& p, std::vector<uint8_t>& out) {
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return false;
    std::fseek(f, 0, SEEK_END); long n = std::ftell(f); std::fseek(f, 0, SEEK_SET);
    out.resize(n > 0 ? n : 0);
    bool ok = n <= 0 || std::fread(out.data(), 1, n, f) == (size_t)n;
    std::fclose(f);
    return ok;
}
void WriteFile(const std::string& p, const std::string& d) {
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) { std::fwrite(d.data(), 1, d.size(), f); std::fclose(f); }
}
} // namespace

int RunShaderTest(const std::string& dir) {
    std::string d = dir;
    while (!d.empty() && (d.back() == '\\' || d.back() == '/')) d.pop_back();
    CreateDirectoryA((d + "\\hlsl").c_str(), nullptr);
    CreateDirectoryA((d + "\\dxil").c_str(), nullptr);
    FILE* log = std::fopen("shader_test.log", "w");
    auto P = [&](const char* fmt, ...) {
        char b[4096];
        va_list ap; va_start(ap, fmt); std::vsnprintf(b, sizeof(b), fmt, ap); va_end(ap);
        if (log) { std::fputs(b, log); std::fflush(log); }
    };
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA((d + "\\ucode_*.bin").c_str(), &fd);
    int total = 0, hlslOk = 0, dxilOk = 0, fail = 0;
    uint64_t aluTotal = 0;
    if (h != INVALID_HANDLE_VALUE) {
        do {
            std::string name = fd.cFileName;
            bool vertex = name.find("_vs.bin") != std::string::npos;
            std::vector<uint8_t> bytes;
            ++total;
            if (!ReadFile(d + "\\" + name, bytes) || bytes.size() < 12 || bytes.size() % 4) { ++fail; P("%s: lectura invalida\n", name.c_str()); continue; }
            ShaderInfo si;
            if (!AnalyzeUcode(vertex, bytes.data(), (uint32_t)(bytes.size() / 4), si)) { ++fail; P("%s: ANALISIS FALLIDO %s\n", name.c_str(), si.error.c_str()); continue; }
            aluTotal += si.aluCount;
            HlslResult hr = TranslateToHlsl(si, HlslOptions());
            std::string stem = name.substr(0, name.size() - 4);
            WriteFile(d + "\\hlsl\\" + stem + ".hlsl", hr.hlsl);
            if (!hr.ok) { ++fail; P("%s: HLSL FALLIDO: %s\n", name.c_str(), hr.error.c_str()); continue; }
            ++hlslOk;
            std::string dxil, err;
            if (!CompileHlslToDxil(hr.hlsl, vertex, dxil, err)) { ++fail; P("%s: DXIL FALLIDO:\n%s\n", name.c_str(), err.c_str()); continue; }
            ++dxilOk;
            WriteFile(d + "\\dxil\\" + stem + ".dxil", dxil);
            P("%s: OK dwords=%u alu=%u fetch=%u (vfetch=%zu tfetch=%zu) regs<=%u dinamicos=%d cf_pares=%u hlsl=%zuB dxil=%zuB%s%s\n", name.c_str(), si.dwords, si.aluCount,
              si.fetchCount, si.vfetch.size(), si.tfetch.size(), si.regBound, (int)si.dynamicRegs, si.cfPairBound, hr.hlsl.size(), dxil.size(),
              err.empty() ? "" : " avisos:", err.empty() ? "" : err.c_str());
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
    char line[256];
    std::snprintf(line, sizeof(line), "shader-test total=%d hlsl_ok=%d dxil_ok=%d fail=%d", total, hlslOk, dxilOk, fail);
    P("%s\n", line);
    std::printf("%s\n", line);
    if (log) std::fclose(log);
    return fail == 0 && total > 0 ? 0 : 1;
}

} // namespace rt::gpu
