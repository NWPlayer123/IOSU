#ifndef _IPC_H_
#define _IPC_H_

/*  http://wiiubrew.org/wiki/IOSU#IPC */
struct ipc_msg {
/*  1:open, 2:close, 3:read, 4:write, 5:seek, 6:ioctl, 7:ioctlv */
    unsigned int cmd;
    unsigned int client_reply;
/*  Invalid if cmd=open */
    unsigned int client_fd;
    unsigned int flags;
/*  0:ARM, 1:PPCcore0, 2:PPCcore1, 3:PPCcore2 */
    unsigned int client_cpu;
    unsigned int client_pid;
    unsigned int client_group_id_upper;
    unsigned int client_group_id_lower;
    unsigned int server_handle;
/*  If cmd = ...
    1:name, 3:outPtr, 4:inPtr, 5:where, 6:cmd, 7:cmd */
    unsigned int arg0;
/*  If cmd = ...
    1:name_size, 3:outLen, 4:inLen, 5:whence, 6:inPtr, 7:readCount */
    unsigned int arg1;
/*  If cmd = ...
    1:mode (0:none, 1:read, 2:write), 6:inLen, 7:writeCount */
    unsigned int arg2;
/*  If cmd = ...
    1:permissions_upper, 6:outPtr, 7:vector */
    unsigned int arg3;
/*  If cmd = ...
    1:permissions_lower, 6:outLen */
    unsigned int arg4;
    unsigned int prev_cmd;
    unsigned int prev_client_fd;
    unsigned int virt0;
    unsigned int virt1;
}

#endif //_IPC_H_