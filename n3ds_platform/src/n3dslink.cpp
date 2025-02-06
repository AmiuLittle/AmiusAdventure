#include "n3dslink.hpp"
#include <3ds.h>
#include <malloc.h>
#include <iostream>
#include "scope_guard.hpp"

#define SOC_ALIGN 0x1000
#define SOC_BUFFERSIZE 0x100000

u32* SOC_buffer = nullptr;
s32 sock = -1;

bool init3dslinkStdio() {
    if (!__3dslink_host.s_addr) return true;

    SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    if (SOC_buffer == nullptr) {
        std::cerr << "Failed to allocate SOC buffer" << std::endl;
        return false;
    }
    auto buff_sg = sg::make_scope_guard([&](){
        free(SOC_buffer);
        SOC_buffer = nullptr;
    });

    u32 ret = socInit(SOC_buffer, SOC_BUFFERSIZE);
    if (ret != 0) {
        std::cerr << "socInit failed " + std::to_string(ret) << std::endl;
        return false;
    }
    auto serv_sg = sg::make_scope_guard([&](){
        socExit();
    });

    sock = link3dsStdio();
    if (sock < 0) {
        std::cerr << "Failed to link stdio" << std::endl;
    }

    buff_sg.dismiss();
    serv_sg.dismiss();
    return true;
}

void exit3dslink() {
    if (sock < 0) return;

    close(sock);
    sock = -1;
    socExit();
    free(SOC_buffer);
}