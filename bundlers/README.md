## docker

First generate the container with:  
```
cd bundlers
docker build -t doccreator .
```

Then run the docker image (on Ubuntu) with:  
`xhost +local:docker`

`sudo docker run -ti --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix/:/tmp/.X11-unix -e QT_X11_NO_MITSHM=1 doccreator /bin/bash`

or  

`sudo docker run -ti --rm  -e "DISPLAY=unix:0.0" -v="/tmp/.X11-unix:/tmp/.X11-unix:rw" --privileged doccreator /bin/bash`

From the docker bash, you can then run the software with:  
`DocCreator-master/build/software/DocCreator/DocCreator`

Warning: the 3D distortion effect will not work properly this way. It is currently only supported when an Nvidia GPU and nvidia-docker2 is used, see below.

### docker with an Nvidia GPU 

Install nvidia-docker2 following the instructions here: https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html#docker  
Basically:
```
sudo systemctl stop docker
sudo apt remove docker-ce docker-ce-cli
curl https://get.docker.com | sh \
  && sudo systemctl --now enable docker


distribution=$(. /etc/os-release;echo $ID$VERSION_ID) \
   && curl -s -L https://nvidia.github.io/nvidia-docker/gpgkey | sudo apt-key add - \
   && curl -s -L https://nvidia.github.io/nvidia-docker/$distribution/nvidia-docker.list | sudo tee /etc/apt/sources.list.d/nvidia-docker.list

sudo apt-get update
sudo apt-get install -y nvidia-docker2
sudo systemctl restart docker

#To check if it is working properly
sudo docker run --rm --gpus all nvidia/cuda:11.0-base nvidia-smi

```

Generate the container the same way:  
```
cd bundlers
docker build -t doccreator .
```

Then run the docker image (on Ubuntu) with:  
`xhost +local:docker`

`sudo docker run -ti --rm --gpus all -e DISPLAY=$DISPLAY -v /tmp/.X11-unix/:/tmp/.X11-unix -e QT_X11_NO_MITSHM=1 doccreator /bin/bash`

Then launch doccreator the same way:  
`DocCreator-master/build/software/DocCreator/DocCreator`

