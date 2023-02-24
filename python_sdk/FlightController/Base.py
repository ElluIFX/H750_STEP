import struct
import threading
import time
import traceback

from .Logger import logger
from .Serial import FC_Serial


def bytes_to_str(data):
    return " ".join([f"{b:02X}" for b in data])


class Byte_Var:
    """
    C-like byte类型变量与python泛型变量的转换类
    使用时直接操作成员bytes和value即可
    """

    _value = 0
    _last_update_time = 0
    _byte_length = 0
    _multiplier = 1
    _signed = False
    _var_type = None
    name = None

    def __set_name__(self, owner, name):
        self.name = name

    def __init__(self, ctype="u8", var_type=int, value_multiplier=1):
        """Args:
        ctype (str): C-like类型(如u8, u16, u32, s8, s16, s32)
        py_var_type (_type_): python类型(如int, float)
        value_multiplier (int, optional): 值在从byte向python转换时的乘数. Defaults to 1.
        """
        self.reset(0, ctype, var_type, value_multiplier)

    def reset(self, init_value, ctype: str, py_var_type, value_multiplier=1):
        """重置变量

        Args:
            init_value (_type_): 初始值(浮点值或整数值)
            ctype (str): C-like类型(如u8, u16, u32, s8, s16, s32)
            py_var_type (_type_): python类型(如int, float)
            value_multiplier (int, optional): 值在从byte向python转换时的乘数. Defaults to 1.
        """
        ctype_word_part = ctype[0]
        ctype_number_part = ctype[1:]
        if ctype_word_part.lower() == "u":
            self._signed = False
        elif ctype_word_part.lower() == "s":
            self._signed = True
        else:
            raise ValueError(f"Invalid ctype: {ctype}")
        if int(ctype_number_part) % 8 != 0:
            raise ValueError(f"Invalid ctype: {ctype}")
        if py_var_type not in [int, float, bool]:
            raise ValueError(f"Invalid var_type: {py_var_type}")
        self._byte_length = int(int(ctype_number_part) // 8)
        self._var_type = py_var_type
        self._multiplier = value_multiplier
        self._value = self._var_type(init_value)
        self._last_update_time = time.perf_counter()
        return self

    @property
    def value(self):
        return self._value

    @value.setter
    def value(self, value):
        self._value = self._var_type(value)
        self._last_update_time = time.perf_counter()

    def update_value_with_mul(self, value):
        self._value = self._var_type(value * self._multiplier)

    @property
    def bytes(self):
        if self._multiplier != 1:
            return int(round(self._value / self._multiplier)).to_bytes(
                self._byte_length, "little", signed=self._signed
            )
        else:
            return int(self._value).to_bytes(
                self._byte_length, "little", signed=self._signed
            )

    @bytes.setter
    def bytes(self, value):
        self._value = self._var_type(
            int.from_bytes(value, "little", signed=self._signed) * self._multiplier
        )
        self._last_update_time = time.perf_counter()

    @property
    def byte_length(self):
        return self._byte_length

    @byte_length.setter
    def byte_length(self, value):
        raise Exception("byte_length is read-only")

    @property
    def last_update_time(self):
        return self._last_update_time

    @last_update_time.setter
    def last_update_time(self, value):
        raise Exception("last_update_time is read-only")

    @property
    def struct_fmt_type(self):
        base_dict = {1: "b", 2: "h", 4: "i", 8: "q"}
        if self._signed:
            return base_dict[self._byte_length]
        else:
            return base_dict[self._byte_length].upper()


class FC_State_Struct:
    step1_speed = Byte_Var("s32", float, 0.01)  # deg/s
    step1_angle = Byte_Var("s32", float, 0.001)  # deg
    step1_target_angle = Byte_Var("s32", float, 0.001)  # deg
    step1_rotating = Byte_Var("u8", bool)  # bool
    step1_dir = Byte_Var("u8", int)  # 0:逆时针 1:顺时针

    step2_speed = Byte_Var("s32", float, 0.01)  # deg/s
    step2_angle = Byte_Var("s32", float, 0.001)  # deg
    step2_target_angle = Byte_Var("s32", float, 0.001)  # deg
    step2_rotating = Byte_Var("u8", bool)  # bool
    step2_dir = Byte_Var("u8", int)  # 0:逆时针 1:顺时针

    step3_speed = Byte_Var("s32", float, 0.01)  # deg/s
    step3_angle = Byte_Var("s32", float, 0.001)  # deg
    step3_target_angle = Byte_Var("s32", float, 0.001)  # deg
    step3_rotating = Byte_Var("u8", bool)  # bool
    step3_dir = Byte_Var("u8", int)  # 0:逆时针 1:顺时针

    RECV_ORDER = [  # 数据包顺序
        step1_speed,step1_angle,step1_target_angle,step1_rotating,step1_dir,
        step2_speed,step2_angle,step2_target_angle,step2_rotating,step2_dir,
        step3_speed,step3_angle,step3_target_angle,step3_rotating,step3_dir,
    ]  # fmt: skip

    def __init__(self):
        self._fmt_string = "<" + "".join([i.struct_fmt_type for i in self.RECV_ORDER])
        self._fmt_length = struct.calcsize(self._fmt_string)

    def update_from_bytes(self, bytes):
        if len(bytes) != self._fmt_length:
            raise ValueError(
                f"Invalid bytes length: {len(bytes)} != {self._fmt_length}"
            )
        vals = struct.unpack(self._fmt_string, bytes)
        for i, val in enumerate(vals):
            self.RECV_ORDER[i].update_value_with_mul(val)


class FC_Event:
    """飞控事件类"""

    def __init__(self):
        self._status = False
        self._callback = None
        self._callback_trigger = True

    def __bool__(self):
        return self._status

    def set(self):
        self._status = True
        self._check_callback()

    def clear(self):
        self._status = False
        self._check_callback()

    def wait(self, timeout=None) -> bool:
        """
        等待事件置位
        Returns:
            bool: True if the event is set, False if the timeout occurred.
        """
        if timeout is None:
            while self._status == False:
                time.sleep(0.1)
        else:
            start_time = time.perf_counter()
            while self._status == False:
                time.sleep(0.1)
                if time.perf_counter() - start_time > timeout:
                    logger.warning("[FC] Wait for event timeout")
                    break
        self._check_callback()
        return self._status

    def wait_clear(self, timeout=None) -> bool:
        ret = self.wait(timeout)
        if ret:
            self.clear()
        return ret

    def _check_callback(self):
        if callable(self._callback) and self._status == self._callback_trigger:
            self._callback()

    def set_callback(self, callback, trigger=True):
        """设置回调函数

        Args:
            callback (function): 目标函数
            trigger (bool, optional): 回调触发方式 (True为事件置位时触发). Defaults to True.
        """
        self._callback = callback
        self._callback_trigger = trigger

    def is_set(self) -> bool:
        return self._status


class FC_Event_Struct:
    key_short = FC_Event()
    key_long = FC_Event()
    key_double = FC_Event()

    EVENT_CODE = {
        0x01: key_short,
        0x02: key_long,
        0x03: key_double,
    }


class FC_Settings_Struct:
    wait_ack_timeout = 0.1  # 应答帧超时时间
    wait_sending_timeout = 0.2  # 发送等待超时时间
    ack_max_retry = 3  # 应答失败最大重发次数
    action_log_output = True  # 是否输出动作日志
    strict_ack_check = True  # 当ACK帧校验失败时抛出异常


class FC_Base_Uart_Comunication(object):
    """
    通讯层, 实现了与飞控的直接串口通讯
    """

    def __init__(self) -> None:
        super().__init__()
        self.running = False
        self.connected = False
        self._start_bit = [0xAA, 0x22]
        self._thread_list = []
        self._state_update_callback = None
        self._print_state_flag = False
        self._ser_32 = None
        self._send_lock = threading.Lock()
        self._recivied_ack_dict = {}
        self._event_update_callback = None  # 仅供FC_Remote使用
        self.state = FC_State_Struct()
        self.event = FC_Event_Struct()
        self.settings = FC_Settings_Struct()

    def start_listen_serial(
        self,
        serial_port: str,
        bit_rate: int = 500000,
        print_state=True,
        callback=None,
    ):
        self._state_update_callback = callback
        self._print_state_flag = print_state
        self._ser_32 = FC_Serial(serial_port, bit_rate)
        self._set_option(0)
        self._ser_32.read_config(startBit=[0xAA, 0x55])
        logger.info("[FC] Serial port opened")
        self.running = True
        _listen_thread = threading.Thread(target=self._listen_serial_task)
        _listen_thread.daemon = True
        _listen_thread.start()
        self._thread_list.append(_listen_thread)

    def quit(self, joined=False) -> None:
        self.running = False
        if joined:
            for thread in self._thread_list:
                thread.join()
                self._thread_list.remove(thread)
        if self._ser_32:
            self._ser_32.close()
        logger.info("[FC] Threads closed, FC offline")

    def _set_option(self, option: int) -> None:
        self._ser_32.send_config(
            startBit=self._start_bit,
            optionBit=[option],
        )

    def send_data_to_fc(
        self,
        data: bytes,
        option: int,
        need_ack: bool = False,
        _ack_retry_count: int = None,
    ):
        """将数据向飞控发送, 并等待应答, 一切操作都将由该函数发送, 因此重构到
        其他通讯方式时只需重构该函数即可

        Args:
            data (bytes): bytes类型的数据
            option (int): 选项, 对应飞控代码
            need_ack (bool, optional): 是否需要应答验证. Defaults to False.
            _ack_retry_count (int, optional): 应答超时时最大重发次数, 此处由函数自动递归设置, 请修改settings中的选项.

        Returns:
            bytes: 实际发送的数据帧
        """
        if need_ack:
            if _ack_retry_count is None:
                _ack_retry_count = self.settings.ack_max_retry
            check_ack = option
            for add_bit in data:
                check_ack = (check_ack + add_bit) & 0xFF
            self._recivied_ack_dict[check_ack] = None
            if _ack_retry_count <= 0:
                logger.error("Wait ACK reached max retry")
                if self.settings.strict_ack_check:
                    raise Exception("Wait ACK reached max retry")
                return None
            send_time = time.perf_counter()
        try:
            self._send_lock.acquire(timeout=self.settings.wait_sending_timeout)
        except:
            logger.error("[FC] Wait sending data timeout")
            return None
        self._set_option(option)
        sended = self._ser_32.write(data)
        self._send_lock.release()
        if need_ack:
            while self._recivied_ack_dict[check_ack] is None:
                if time.perf_counter() - send_time > self.settings.wait_ack_timeout:
                    logger.warning(f"[FC] ACK timeout, retry - {_ack_retry_count}")
                    return self.send_data_to_fc(
                        data, option, need_ack, _ack_retry_count - 1
                    )
                time.sleep(0.001)
            self._recivied_ack_dict.pop(check_ack)
        return sended

    def _listen_serial_task(self):
        logger.info("[FC] listen serial thread started")
        last_heartbeat_time = time.perf_counter()
        last_receive_time = time.perf_counter()
        while self.running:
            try:
                if self._ser_32.read():
                    last_receive_time = time.perf_counter()
                    _data = self._ser_32.rx_data
                    cmd = _data[0]
                    data = _data[1:]
                    if cmd == 0x01:  # 状态回传
                        self._update_state(data)
                    elif cmd == 0x02:  # ACK返回
                        self._recivied_ack_dict[data[0]] = time.perf_counter()
                    elif cmd == 0x03:  # 事件通讯
                        self._update_event(data)
                if time.perf_counter() - last_heartbeat_time > 0.25:  # 心跳包
                    self.send_data_to_fc(b"\x01", 0x00)
                    last_heartbeat_time = time.perf_counter()
                if time.perf_counter() - last_receive_time > 0.5:  # 断连检测
                    if self.connected:
                        self.connected = False
                        logger.warning("[FC] Disconnected")
                pop_list = []
                for ack, recv_time in self._recivied_ack_dict.items():  # 超时ACK清理
                    if recv_time is not None and time.perf_counter() - recv_time > 0.5:
                        pop_list.append(ack)
                if len(pop_list) > 0:
                    logger.warning(f"[FC] Removed {len(pop_list)} unrecognized ACK")
                    for ack in pop_list:
                        try:
                            self._recivied_ack_dict.pop(ack)
                        except:
                            pass
                time.sleep(0.001)  # 降低CPU占用
            except Exception as e:
                logger.error(f"[FC] listen serial exception: {traceback.format_exc()}")

    def _update_state(self, recv_byte):
        try:
            # index = 0
            # for var in self.state.RECV_ORDER:
            #     length = var.byte_length
            #     var.bytes = recv_byte[index : index + length]
            #     index += length
            self.state.update_from_bytes(recv_byte)
            if not self.connected:
                self.connected = True
                logger.info("[FC] Connected")
            if callable(self._state_update_callback):
                self._state_update_callback(self.state)
            if self._print_state_flag:
                self._print_state()
        except Exception as e:
            logger.error(f"[FC] Update state exception: {traceback.format_exc()}")

    def _set_event_callback(self, func):
        self._event_update_callback = func

    def _update_event(self, recv_byte):
        try:
            event_code = recv_byte[0]
            event_operator = recv_byte[1]
            if event_operator == 0x01:  # set
                self.event.EVENT_CODE[event_code].set()
                logger.debug(f"[FC] Event {event_code} set")
            elif event_operator == 0x02:  # clear
                self.event.EVENT_CODE[event_code].clear()
                logger.debug(f"[FC] Event {event_code} clear")
            if callable(self._event_update_callback):
                self._event_update_callback(event_code, event_operator)
        except Exception as e:
            logger.error(f"[FC] Update event exception: {traceback.format_exc()}")

    def _print_state(self):
        import re

        RED = "\033[1;31m"
        GREEN = "\033[1;32m"
        YELLOW = "\033[1;33m"
        BLUE = "\033[1;34m"
        CYAN = "\033[1;36m"
        PURPLE = "\033[1;35m"
        RESET = "\033[0m"
        BACK = "\033[F"
        LINELIMIT = 100  # 每行最多显示的字符数
        LOG_SPACE = 3  # 为日志留出的空间
        BOXCOLOR = BLUE
        HEAD = f"{BOXCOLOR}| {RESET}"
        TAIL = f"{BOXCOLOR} |{RESET}"
        lines = [
            BOXCOLOR
            + "-" * ((LINELIMIT - 32) // 2)
            + f" ▲ System log / ▼ System status "
            + "-" * ((LINELIMIT - 32) // 2)
            + RESET,
            HEAD,
        ]

        def remove_color(text):
            return re.sub(r"\033\[[0-9;]*m", "", text)

        def len_s(text):
            return len(remove_color(text))

        varlist = [
            f"{YELLOW}{var.name}: {f'{GREEN}√ ' if var.value else f'{RED}x {RESET}'}"
            if type(var.value) == bool
            else (
                f"{YELLOW}{var.name}:{CYAN}{var.value:^7.02f}{RESET}"
                if type(var.value) == float
                else f"{YELLOW}{var.name}:{CYAN}{var.value:^4d}{RESET}"
            )
            for var in self.state.RECV_ORDER
        ]
        for vartext in varlist:
            if len_s(lines[-1]) + len_s(vartext) > LINELIMIT - 2:
                lines[-1] += " " * (LINELIMIT - len_s(lines[-1]) - 2) + TAIL
                lines.append(HEAD)
            lines[-1] += vartext
        lines[-1] += " " * (LINELIMIT - len_s(lines[-1]) - 2) + TAIL
        lines.append(f"{BOXCOLOR}{'-' * LINELIMIT}{RESET}")
        for _ in range(LOG_SPACE):
            lines.insert(0, " " * LINELIMIT)
        text = "\n".join(lines) + BACK * (len(lines) - 1)
        print(text, end="")
