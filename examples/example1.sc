
#let host_ip_address = "10.100.32.140"
let host_ip_address = "192.168.100.24"

let client = -1
let E5Model = "EqModel"
let E5SoftwareVersion = "1.2.14"
let sc_version = get_version()

print "SC Version is <" . $sc_version . ">"

open socket_server name=host_connection proto=hsms logging=2 host=192.168.100.24 service=5000 options="t3=5 t6=10 t7=10 t8=10 timeout_msg=TIMEOUT" 


# All communications from the host start here
label host_connection
    let client = get_client(host_connection)
    let sxfy= get_tmp_var(SXFY)
    goto $sxfy

# S1F1 - Hello from the host
label S1F1
    send_reply name=host_connection S1F2 0
    <L
        <A $E5Model>
        <A $E5SoftwareVersion>
    > .

# S1F13 - Establish Communications, CommunicationState is COMMUNICATING
label S1F13
    send_reply name=host_connection S1F14 0
    <L
        <B 00>
        <L
            <A $E5Model>
            <A $E5SoftwareVersion>
        >
    >.

# S7F19 - Request PPID List
label S7F19
    send_reply name=host_connection S7F20 0
    <L
        <A '/Test/Recipe001'>
        <A '/Test2/Recipe002'>
        <A 'SZ41T_8a76A'>
        <A 'EMPTY RECIPE'>
    > .

label run_job
    # send job started event
    send name=host_connection client=$client S6F11 0 W
    <L
        <U4 23>
        <U4 5458>
        <L
            <L
                <U4 106>
                <L
                    <A 'CJ001'>
                >
            >
        >
    > .

    sleep 10

    # send job completed event
    send name=host_connection client=$client S6F11 0 W
    <L
        <U4 24>
        <U4 5459>
        <L
            <L
                <U4 106>
                <L
                    <A 'CJ001'>
                >
            >
        >
    > .


