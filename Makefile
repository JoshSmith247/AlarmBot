INC = -Icommon -Icontrol -Icomms
SRC = common/protocol.c common/link.c control/fsm.c control/lift.c control/ctl.c comms/app.c comms/cmd.c comms/sched.c
CFLAGS = -Wall -Wextra -Werror -g

test:
	mkdir -p build
	cc $(CFLAGS) $(INC) common/protocol.c common/test_protocol.c -o build/test_protocol && build/test_protocol
	cc $(CFLAGS) $(INC) tests/test_e2e.c $(SRC) -lm -o build/test_e2e && build/test_e2e
