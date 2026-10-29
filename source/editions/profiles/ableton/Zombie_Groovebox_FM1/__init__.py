# SPDX-License-Identifier: MIT
from .surface import ZombieCoreFM1


def create_instance(c_instance):
    return ZombieCoreFM1(c_instance)
