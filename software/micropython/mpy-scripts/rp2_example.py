import concentrator
import socket
# import machine

# machine.sleep()
print(concentrator.add_ints(1, 3))


def myfct():
    print("mycallback")

concentrator.set_callback(myfct)
concentrator.call_callback()