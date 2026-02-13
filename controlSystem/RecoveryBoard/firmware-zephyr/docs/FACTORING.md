# ERS Zephyr Firmware

Document regarding ERS Zephyr firmware source code factoring.

## Purpose

This document started to track how the ERS Zephyr based firmware is factored, at
a module level and file level.

## Issues

- (1) Decision points not fully encapsulated in arbiter module


Issue (1)

There's an arbiter module, but the arbiter does not make all of the decisions,
which would make the control flow at a practical and higher level easier to
read and to understand.  An example is the factoring of certain GPIO line reads
whose results more or less get put into the summary CAN message frame that's
published once a second.  The reading from that line aside from needing to be
inverted doesn't require any logic.  It simply needs to be written to the
ERS Drogue or Main chute CAN "heartbeat" message.

QUESTION:
How can the code be self-explanatory for this?

QUESTION:
How may we comment the code to indicate a reading with no required processing
falls outside the arbiter?  (No arbitration or decision making needed for it.)

ANSWER:
Create top-of-file comment block in arbiter module and describe each datum
which must be figured out through some kind of decision making process.
