import threading
import time

import numpy as np

from .Logger import logger
from .Protocal import FC_Protocol


class FC_Application(FC_Protocol):
    """
    应用层, 基于协议层进行开发, 不触及底层通信
    """

    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)

    def wait_for_connection(self, timeout_s=-1) -> bool:
        """
        等待飞控连接
        """
        t0 = time.time()
        while not self.connected:
            time.sleep(0.1)
            if timeout_s > 0 and time.time() - t0 > timeout_s:
                logger.warning("[FC] wait for fc connection timeout")
                return False
        self._action_log("wait ok", "fc connection")
        return True

    def wait_for_step_idle(self, motor: int, timeout_s=30) -> bool:
        """
        等待电机停止转动
        """
        t0 = time.time()
        time.sleep(0.1)  # 等待数据回传

        while not self.step_idle(motor):
            time.sleep(0.1)
            if timeout_s > 0 and time.time() - t0 > timeout_s:
                logger.warning(f"[FC] wait for step {motor} idle timeout")
                return False
        self._action_log("wait ok", f"Step {motor} idle")
        return True

    def set_action_log(self, output: bool) -> None:
        """
        设置动作日志输出
        """
        self.settings.action_log_output = output
