#pragma once

struct xdp_program *load_bpf_and_xdp_attach();
void redirect_rule_set(struct xdp_program *prog);