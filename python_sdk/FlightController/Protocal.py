import struct
import time

from .Base import Byte_Var, FC_Base_Uart_Comunication, bytes_to_str
from .Logger import logger


class FC_Protocol(FC_Base_Uart_Comunication):
    """
    协议层, 定义了实际的控制命令
    """

    # masks
    STEP1 = 0x01
    STEP2 = 0x02
    STEP3 = 0x04

    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self._byte_temp1 = Byte_Var()
        self._byte_temp2 = Byte_Var()
        self._byte_temp3 = Byte_Var()
        self._byte_temp4 = Byte_Var()

    def _action_log(self, action: str, data_info: str = None):
        if self.settings.action_log_output:
            string = f"[FC] [ACTION] {action.upper()}"
            if data_info is not None:
                string += f" -> {data_info}"
            logger.info(string)

    ######### 飞控命令 #########

    def _send_command(self, option: int, data: bytes = b"", need_ack=True) -> None:
        sended = self.send_data_to_fc(data, option, need_ack=need_ack)
        # logger.debug(f"[FC] Send: {bytes_to_str(sended)}")

    def step_set_speed(self, motor: int, speed: float):
        """
        设置电机速度
        motor: 电机掩码(eg: STEP1 | STEP2)
        speed: deg/s, 正数
        """
        self._byte_temp1.reset(motor, "u8", int)
        self._byte_temp2.reset(speed, "s32", float, 0.01)
        self._send_command(0x01, self._byte_temp1.bytes + self._byte_temp2.bytes)

    def step_set_angle(self, motor: int, angle: float):
        """
        设置电机当前角度
        motor: 电机掩码(eg: STEP1 | STEP2)
        angle: deg 当前角度, 正数为顺时针
        """
        self._byte_temp1.reset(motor, "u8", int)
        self._byte_temp2.reset(angle, "s32", float, 0.01)
        self._send_command(0x02, self._byte_temp1.bytes + self._byte_temp2.bytes)

    def step_rotate(self, motor: int, deg: float):
        """
        相对旋转电机
        motor: 电机掩码(eg: STEP1 | STEP2)
        deg: deg 旋转角度, 正数为顺时针
        """
        self._byte_temp1.reset(motor, "u8", int)
        self._byte_temp2.reset(deg, "s32", float, 0.001)
        self._send_command(0x03, self._byte_temp1.bytes + self._byte_temp2.bytes)

    def step_rotate_abs(self, motor: int, deg: float):
        """
        绝对旋转电机
        motor: 电机掩码(eg: STEP1 | STEP2)
        deg: deg 旋转到的绝对角度, 正数为顺时针
        """
        self._byte_temp1.reset(motor, "u8", int)
        self._byte_temp2.reset(deg, "s32", float, 0.001)
        self._send_command(0x04, self._byte_temp1.bytes + self._byte_temp2.bytes)
