import httpx
import streamlit as st

TV_IR_HOST = "192.168.1.113"

st.title("TV IR")


def send_ir(type: str, hex_code: str):
    """赤外線の送信"""
    res = httpx.post(
        f"http://{TV_IR_HOST}/ir/send",
        json={
            "type": "SONY",
            "hex": hex_code,
        },
    )
    st.json(res.json())
    if res.status_code == 200:
        st.success("Success")
    else:
        st.error(f"Failed status: {res.status_code}")


def receive_ir():
    """赤外線の受信"""
    res = httpx.get(f"http://{TV_IR_HOST}/ir/receive", timeout=15)
    st.json(res.json())
    if res.status_code == 200:
        st.success("Success")
    else:
        st.error(f"Failed status: {res.status_code}")


if st.button("TV Power"):
    send_ir("SONY", "0xA90")

if st.button("receive"):
    receive_ir()
