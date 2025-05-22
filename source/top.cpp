#include "top.h"

top::top(sc_module_name name)
    : sc_module(name),
      clk("clk_4", 250.0, SC_NS, 125.0, 0, SC_MS, true),
      bridge_obj("qemu_bridge", true, "/tmp/fake_qemu.sock")
{
    bridge_obj.clk(clk);
}