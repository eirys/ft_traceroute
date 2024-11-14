FROM debian:latest

RUN apt update -y \
&& apt install -yq \
# For debugging
valgrind \
man \
less \
# Traceroute
traceroute \
# Wireshark
wireshark \
&& apt clean -y

# WORKDIR /home/trace_output
WORKDIR /home

CMD ["tail", "-f", "/dev/null"]