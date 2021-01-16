FROM hello_crow
WORKDIR /usr/src/cppweb/hello_crow/build
COPY  .  .
CMD ["./build/hello_crow"]
