from datetime import datetime

import streamlit as st

st.title("Title")
st.write(datetime.now())

ok = st.button("run")
if not ok:
    st.stop()

st.write("push run")
