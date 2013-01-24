ifndef GUBG
$(error You have to specify where gubg is to be found)
endif
ifndef GUBG_SDKS
$(error You have to specify where the sdks are to be found)
endif
ifndef GUBG_TMP
$(error You have to specify where the tmp is to be found)
endif
$(info "GUBG: $(GUBG)")
$(info "GUBG_SDKS: $(GUBG_SDKS)")
$(info "GUBG_TMP: $(GUBG_TMP)")

.PHONY: env
env: $(GUBG_SDKS) $(GUBG_TMP)

$(GUBG_SDKS):;mkdir $@
$(GUBG_TMP):;mkdir $@
