import argparse
from dataclasses import dataclass
from typing import List

@dataclass
class Partition:
    type_code: int
    lba_begin: int


def read_partitions_from_mbr(mbr: bytes) -> List[Partition]:
    partitions = []
    for i in range(4):
        offset = 446 + i * 16
        partition = mbr[offset:offset+16]
        assert len(partition) == 16
        type_code = partition[4]
        lba_begin = int.from_bytes(partition[8:12], byteorder='little', signed=False)
        partitions.append(Partition(type_code, lba_begin))
    return partitions


def check_disk_mbr(mbr: bytes):
    assert mbr[-2:] == bytes.fromhex("55AA")
    # print(f"Disk \"{disk_image_path}\" partition table:")
    partitions = read_partitions_from_mbr(mbr)
    for i, partition in enumerate(partitions):
        # print(f"    Partition {i+1} type={partition.type_code} lba_begin={partition.lba_begin}")
        if partition.type_code != 0:
            # partition is present
            assert partition.lba_begin >= 2048, "partition is present on the disk too early, can't write kernel code"


def check_boot_code(boot_code: bytes):
    assert boot_code[-2:] == bytes.fromhex("55AA")
    assert boot_code[446:-2] == bytes([0] * 64), "boot code must contain zeros in the place for partition table"


def check_kernel_code(kernel_code: bytes):
    assert len(kernel_code) <= 1024 ** 2 - 512, "kernel code must have size 1MiB - 512 bytes for MBR"
    assert len(kernel_code) < 63 * 1024, "right now we load only first 63.5 KiB of kernel code"


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Create bootable disk image with operating system")
    parser.add_argument("disk.img", help="Path to disk image")
    parser.add_argument("boot.bin", help="Path to executable that will be written to MBR")
    parser.add_argument("kernel.bin", help="Path to kernel executable")
    args: argparse.Namespace = parser.parse_args()
    disk_image_path = getattr(args, "disk.img")
    boot_path = getattr(args, "boot.bin")
    kernel_path = getattr(args, "kernel.bin")

    with open(disk_image_path, "rb") as f:
        mbr = f.read(512)
        check_disk_mbr(mbr)

    with open(boot_path, "rb") as f:
        boot_code = f.read()
        check_boot_code(boot_code)


    with open(kernel_path, "rb") as f:
        kernel_code = f.read()
        check_kernel_code(kernel_code)

    bytes_to_flush = bytearray(1024 ** 2)  # 1 MiB
    bytes_to_flush[0:446] = boot_code[0:446]
    bytes_to_flush[446:512] = mbr[446:512]
    bytes_to_flush[512:] = kernel_code

    with open(disk_image_path, 'r+b') as f:
        f.write(bytes_to_flush)  # Write to start of the disk image
