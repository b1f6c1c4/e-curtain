TOOLCHAIN?=$(HOME)/x-tools/armv6-rpi-linux-gnueabihf
DIR=cmake-build-release-arm
DEBUGGER_DIR=cmake-build-release

export TOOLCHAIN
export DIR

build:
	cmake -S . -B $(DIR)
	$(MAKE) -C $(DIR)

$(DEBUGGER_DIR)/debugger: FORCE
	TOOLCHAIN= cmake -S . -B $(DEBUGGER_DIR)
	TOOLCHAIN= $(MAKE) -C $(DEBUGGER_DIR) debugger

FORCE:

deploy: deploy-0 deploy-1 deploy-2

undeploy: undeploy-0 undeploy-1 undeploy-2

deploy-0: build
	script/deploy.sh F-G0 $(HOST0) $(HOST2)

log-0:
	ssh pi@$(HOST0) sudo journalctl -u F-G0

debug-0: $(DEBUGGER_DIR)/debugger
	$< $(HOST0)

undeploy-0:
	script/undeploy.sh F-G0 $(HOST0)

deploy-1: build
	script/deploy.sh H1-G1 $(HOST1) $(HOST2)

log-1:
	ssh pi@$(HOST1) sudo journalctl -u H1-G1

debug-1: $(DEBUGGER_DIR)/debugger
	$< $(HOST1)

undeploy-1:
	script/undeploy.sh H1-G1 $(HOST1)

deploy-2: build
	script/deploy.sh H2 $(HOST2) $(HOST2)
	script/deploy.sh H0-C $(HOST2) $(HOST0) $(HOST1)

log-2:
	ssh pi@$(HOST2) sudo journalctl -u H2 -u H0-C

debug-2: $(DEBUGGER_DIR)/debugger
	$< $(HOST2)

undeploy-2:
	script/undeploy.sh H2 $(HOST2)
	script/undeploy.sh H0-C $(HOST2)
