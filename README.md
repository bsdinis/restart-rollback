# Restart-Rollback

Code repositories from [RR: A Fault Model for Efficient TEE Replication](http://bsdinis.site/papers/pdf/rr_ndss2023.pdf).

- `register-sgx`: implements a R/W distributed register in three fault models (CFT, BFT and RR);
- `smr-sgx`: implements a replicated state machine in three fault models (CFT, BFT and RR);
- `teems`: implements TEEMS (TEE metadata service), a fault tolerant TEE-based metadata service for enhancing cloud storage with TEE-like confidentiality and integrity properties.
