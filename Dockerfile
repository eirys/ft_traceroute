FROM debian:latest

COPY ./ft_traceroute /home/ft_traceroute

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

WORKDIR /home

CMD ["tail", "-f", "/dev/null"]