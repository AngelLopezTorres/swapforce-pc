// Ejecutable autonomo para iterar sobre el banco de pruebas sin compilar los 482 ficheros recompilados (ver build_standalone.cmd).
#include <cstdint>
#include <cstdlib>
int RunCpuSelfTest(uint64_t n, uint64_t seed);
int main(int argc, char** argv) { return RunCpuSelfTest(argc > 1 ? std::strtoull(argv[1], nullptr, 0) : 10000, argc > 2 ? std::strtoull(argv[2], nullptr, 0) : 1); }
