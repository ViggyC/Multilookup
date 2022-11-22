# Multilookup

This is the repository for CSCI3753 PA3
This is a multithreaded C program that uses requester threads to read IP addresses
and resolver threads to fetch the domain names of these IP addresses and
stores them in a text file.
The core of this synchronization problem is the Bounded Buffer 

# Running locally

git clone
make 
./multilookup #requesters #resolvers res.txt req.txt input/*
