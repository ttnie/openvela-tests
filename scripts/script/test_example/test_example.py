#!/usr/bin/python3
############################################################################
# tests/scripts/script/test_example/test_example.py
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################
# encoding: utf-8
import pytest

pytestmark = [pytest.mark.common, pytest.mark.qemu]


def test_hello(p):
    ret = p.sendCommand("hello", "Hello, World!!")
    assert ret == 0


def test_helloxx(p):
    ret = p.sendCommand("helloxx", "Hello, World!!")
    assert ret == 0


def test_pipe(p):
    ret = p.sendCommand("pipe", "redirect_reader: Returning success", timeout=60)
    assert ret == 0


def test_popen(p):
    ret = p.sendCommand("popen", "Calling pclose()")
    assert ret == 0


@pytest.mark.skip
def test_usrsocktest(p):
    ret = p.sendCommand("usrsocktest", "FAILED:0", timeout=60)
    assert ret == 0
