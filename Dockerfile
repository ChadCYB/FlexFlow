FROM nvidia/cuda:11.1.1-cudnn8-devel-ubuntu18.04

RUN apt-get update && apt-get install -y --no-install-recommends wget sudo binutils git zlib1g-dev libhdf5-dev && \
    rm -rf /var/lib/apt/lists/*

RUN wget -c https://repo.anaconda.com/miniconda/Miniconda3-py38_4.8.3-Linux-x86_64.sh && \
    chmod +x Miniconda3-py38_4.8.3-Linux-x86_64.sh && \
    ./Miniconda3-py38_4.8.3-Linux-x86_64.sh -b -p /opt/conda && \
    rm Miniconda3-py38_4.8.3-Linux-x86_64.sh && \
    /opt/conda/bin/conda upgrade --all && \
    /opt/conda/bin/conda install conda-build conda-verify && \
    /opt/conda/bin/conda clean -ya

RUN /opt/conda/bin/conda install cmake make pillow
RUN /opt/conda/bin/conda install -c conda-forge \
        python=3.8 \
        protobuf=3.9.2 \
        numpy \
        keras-preprocessing \
        pybind11=2.6.2

ENV PATH=/opt/conda/bin:$PATH
ENV CUDNN_DIR=/usr/local/cuda
ENV CUDA_DIR=/usr/local/cuda
ENV PROTOBUF_DIR=/opt/conda/pkgs/libprotobuf-3.9.2-h8b12597_0
ENV LD_LIBRARY_PATH=$PROTOBUF_DIR/lib:$LD_LIBRARY_PATH

# RUN cd /usr && \
    # git clone --recursive https://github.com/ChadCYB/FlexFlow
COPY . /usr/FlexFlow

# RUN cd /usr/FlexFlow && \
#     wget -qO- https://github.com/StanfordLegion/legion/archive/refs/tags/legion-23.06.0.tar.gz | tar -xz && \
#     mv legion-legion-23.06.0 legion

ENV FF_HOME=/usr/FlexFlow
ENV LG_RT_DIR=/usr/FlexFlow/legion/runtime
ENV NVIDIA_VISIBLE_DEVICES=all
ENV NVIDIA_DRIVER_CAPABILITIES=compute,utility

RUN cd /usr/FlexFlow/python && \
    make -j4

RUN ln -s /usr/local/cuda-11.1/compat/libcuda.so.1 /usr/local/cuda/lib64/ && \
    cd /usr/FlexFlow/examples/cpp/DLRMsim && \
    make -j4

WORKDIR /usr/FlexFlow

CMD ["/bin/bash"]