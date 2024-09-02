#pragma once

struct bpf_object *load_bpf_and_tc_attach();
void redirect_rule_set(struct bpf_object *obj);
void tc_dettach(struct bpf_object *obj);