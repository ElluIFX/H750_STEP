import struct
import time

from .Base import Byte_Var, FC_Base_Uart_Comunication
from .Logger import logger


class FC_Protocol(FC_Base_Uart_Comunication):
    """
    协议层, 定义了实际的控制命令
    """

    # constants

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

    def set_rgb_led(self, r: int, g: int, b: int) -> None:
        """
        设置由32控制的RGB LED
        r,g,b: 0-255
        """
        self._byte_temp1.reset(r, "u8", int)
        self._byte_temp2.reset(g, "u8", int)
        self._byte_temp3.reset(b, "u8", int)
        self._send_command(
            0x01,
            self._byte_temp1.bytes + self._byte_temp2.bytes + self._byte_temp3.bytes,
            True,
        )
        self._action_log("set rgb led", f"#{r:02X}{g:02X}{b:02X}")
