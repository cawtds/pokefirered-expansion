FROM python:3.11

WORKDIR /home/
COPY install-devkitpro-pacman ./install-devkitpro-pacman
COPY entrypoint.sh ./entrypoint.sh

RUN ["/bin/bash", "-c", "apt update && apt install -y build-essential gcc-arm-none-eabi binutils-arm-none-eabi git vim libpng-dev python3 wget"]
RUN ["/bin/bash", "-c", "git clone https://github.com/pret/agbcc"]
RUN ["/bin/bash", "-c","chmod +x ./install-devkitpro-pacman && ./install-devkitpro-pacman"]
RUN ["dkp-pacman", "-Sy"]
RUN ["/bin/bash", "-c", "ln -s /proc/self/mounts /etc/mtab && dkp-pacman -S gba-dev --noconfirm"]
COPY . /home/pokefirered
RUN ["/bin/bash", "-c", "cd /home/agbcc/ && ./build.sh && ./install.sh ../pokefirered/"]

WORKDIR /home/pokefirered

ENTRYPOINT ["make"]
