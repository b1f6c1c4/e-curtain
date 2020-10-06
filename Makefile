TOOLCHAIN?=$(HOME)/x-tools/armv6-rpi-linux-gnueabihf
DIR=cmake-build-release-arm

export TOOLCHAIN
export DIR

build:
	cmake -S . -B $(DIR)
	$(MAKE) -C $(DIR)

deploy: deploy-0 deploy-1 deploy-2

undeploy: undeploy-0 undeploy-1 undeploy-2

deploy-0: build
	script/deploy.sh F-G0

log-0:
	ssh pi@$(HOST0) sudo journalctl -u F-G0

undeploy-0:
	script/undeploy.sh F-G0

deploy-1: build
	script/deploy.sh H1-G1

log-1:
	ssh pi@$(HOST1) sudo journalctl -u H1-G1

undeploy-1:
	script/undeploy.sh H1-G1

deploy-2: build
	script/deploy.sh H2
	script/deploy.sh H0-C

log-2:
	ssh pi@$(HOST2) sudo journalctl -u H2 -u H0-C

undeploy-2:
	script/undeploy.sh H2
	script/undeploy.sh H0-C
