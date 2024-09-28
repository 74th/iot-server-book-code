import streamlit as st
from datetime import datetime

st.title("Title")
st.write(datetime.now())

ok = st.button("run")
if not ok:
    st.stop()

st.write("push run")