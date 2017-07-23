#!/bin/sh
kill -9 `top | grep bbsd |grep RUN | awk '{print $1}'`
kill -9 `top | grep bbsd |grep CPU | awk '{print $1}'`
