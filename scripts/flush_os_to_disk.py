import argparse
from dataclasses import dataclass
from typing import List
import subprocess
import tempfile
import os


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
    assert len(mbr) == 512, f"length of MBR must be 512 bytes, not {len(mbr)}"
    assert mbr[-2:] == bytes.fromhex("55AA")
    # print(f"Disk partition table:")
    partitions = read_partitions_from_mbr(mbr)
    for i, partition in enumerate(partitions):
        # print(f"    Partition {i+1} type={partition.type_code} lba_begin={partition.lba_begin}")
        if partition.type_code != 0:
            # partition is present
            assert partition.lba_begin >= 2048, "partition is present on the disk too early, can't write kernel code"


def check_boot_code(boot_code: bytes):
    assert boot_code[-2:] == bytes.fromhex("55AA")
    assert boot_code[446:-2] == bytes([0] * 64), "boot code must contain zeros in the place for partition table"
    assert len(boot_code) == 512, "boot code must be 512 bytes long"


def check_kernel_code(kernel_code: bytes):
    assert len(kernel_code) <= 1024 ** 2 - 512, "kernel code must have size 1MiB - 512 bytes for MBR"
    assert len(kernel_code) < 63 * 1024, "kernel is too big: right now we load only first 63.5 KiB of kernel code"

def shell(command: str):
    subprocess.run(command, shell=True, check=True)


def create_disk_image(path: str, userspace_paths: List[str]):
    # mformat -i $@ -F ::
	# mmd -i $@ ::/boot
	# mcopy -i $@ $< ::/boot
    with tempfile.TemporaryDirectory() as temp_dir:
        fat32_path = os.path.join(temp_dir, "fat32.img")
        hello_path = os.path.join(temp_dir, "hello.txt")

        with open(os.path.join(temp_dir, "hello.txt"), "w") as f:
            f.write("Henlo World!!\n")

        shell(f"dd if=/dev/zero of={fat32_path} bs=1M count=39 status=none")
        shell(f"mformat -c 2 -i {fat32_path} -F ::")
        shell(f"mmd -i {fat32_path} ::/home")
        shell(f"mmd -i {fat32_path} ::/usr")
        shell(f"mcopy -i {fat32_path} {hello_path} ::/home")
        for path in userspace_paths:
            filename = os.path.basename(path)
            shell(f"mcopy -i {fat32_path} {filename} ::/usr")

        with open(fat32_path, "rb") as f:
            fat32_partition_bytes = f.read()

    bytes_to_flush = bytearray(1 * 1024 ** 2)         # 1 MiB
    bytes_to_flush += fat32_partition_bytes       # 39 MiB
    assert len(bytes_to_flush) == 40 * 1024 ** 2  # 40 MiB
    # write MBR signature
    bytes_to_flush[510:512] = bytes.fromhex("55AA")
    # write parition table
    partition = bytearray(16)
    partition[4] = 0x0C  #  FAT32 with LBA addressing
    lba_start = 2048  # after partition gap
    number_of_sectors = len(fat32_partition_bytes) // 512
    assert len(fat32_partition_bytes) % 512 == 0
    partition[8:12] = lba_start.to_bytes(4, byteorder="little", signed=False)
    partition[12:16] = number_of_sectors.to_bytes(4, byteorder="little", signed=False)
    bytes_to_flush[446:446+16] = partition
    with open(path, "wb") as f:
        f.write(bytes_to_flush)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Create bootable disk image with operating system")
    parser.add_argument("--recreate", action="store_true")
    parser.add_argument("disk.img", help="Path to disk image")
    parser.add_argument("boot.bin", help="Path to executable that will be written to MBR")
    parser.add_argument("kernel.bin", help="Path to kernel executable")
    args: argparse.Namespace = parser.parse_args()
    disk_image_path: str = getattr(args, "disk.img")
    boot_path: str = getattr(args, "boot.bin")
    kernel_path: str = getattr(args, "kernel.bin")
    recreate: bool = getattr(args, "recreate")

    if recreate or not os.path.exists(disk_image_path):
        print("Creating disk image")
        create_disk_image(disk_image_path)

    with open(disk_image_path, "rb") as f:
        mbr = f.read(512)
        check_disk_mbr(mbr)
    with open(boot_path, "rb") as f:
        boot_code = f.read()
        check_boot_code(boot_code)
    with open(kernel_path, "rb") as f:
        kernel_code = f.read()
        check_kernel_code(kernel_code)

    bytes_to_flush = bytearray(1024 ** 2)  # only first 1 MiB is altered
    bytes_to_flush[0:446] = boot_code[0:446]
    bytes_to_flush[446:512] = mbr[446:512]
    bytes_to_flush[512:512+len(kernel_code)] = kernel_code

    with open(disk_image_path, 'r+b') as f:
        f.write(bytes_to_flush)  # Write to start of the disk image

    print("OS successfully flushed to the disk")
