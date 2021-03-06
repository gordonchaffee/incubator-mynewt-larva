/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <shell/shell.h>
#include <elua_base/elua.h>

#ifdef MYNEWT

#ifdef SHELL_PRESENT
static struct shell_cmd lua_shell_cmd;

static int
lua_cmd(int argc, char **argv)
{
    lua_main(argc, argv);
    return 0;
}
#endif

int
lua_init(void)
{
#ifdef SHELL_PRESENT
    return shell_cmd_register(&lua_shell_cmd, "lua", lua_cmd);
#else
    return 0;
#endif
}
#endif
