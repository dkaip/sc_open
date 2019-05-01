
#open socket_client name=tool_connection proto=hsms logging=2 host=10.100.32.140 service=5000 options="t3=5 t6=10 t7=10 t8=10 timeout_msg=TIMEOUT"
open socket_client name=tool_connection proto=hsms logging=2 host=192.168.100.24 service=5000 options="t3=5 t6=10 t7=10 t8=10 timeout_msg=TIMEOUT"

label tool_connection
    let sxfy = get_tmp_var(SXFY)
    goto $sxfy

# S1F13 - Establish Communications, CommunicationState is COMMUNICATING
# This is what the response to an S1F13 is when returned from a “HOST”
label S1F13
    send_reply name=tool_connection S1F14 0
    <L
        <B 00>
        <L
        >
    >.

label est_com
    send name=tool_connection S1F13 0 W
    <L>.

label hello
    send name=tool_connection S1F1 0 W
    <L>.

label offline
    send name=tool_connection S1F15 0 W .

label online
    send name=tool_connection S1F17 0 W .

label get_ppid_list
    send name=tool_connection S7F19 0 W .

label S6F11
    send_reply name=tool_connection S6F12 0
    <B 00>.
