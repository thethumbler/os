#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MBR_STATUS_NON_BOOTABLE     0x00
#define MBR_STATUS_NON_BOOTABLE_LBA 0x01
#define MBR_STATUS_BOOTABLE         0x80
#define MBR_STATUS_BOOTABLE_LBA     0x81

#define MBR_TYPE_UNUSED             0x00
#define MBR_TYPE_EXTENDED_DOS       0x05
#define MBR_TYPE_NTFS               0x07
#define MBR_TYPE_EXTENDED_WINDOWS   0x0F
#define MBR_TYPE_LINUX_SWAP         0x82
#define MBR_TYPE_LINUX              0x83

#define MBR_COPY_PROTECTED          0x5A5A

#define MBR_BOOT_SIGNATURE          0xAA55

#pragma pack(1)

typedef struct {
    uint8_t   status;
    struct {
        uint8_t   h;
        uint16_t  cs;
    } start_chs;
    uint8_t   type;
    struct {
        uint8_t   h;
        uint16_t  cs;
    } end_chs;
    uint32_t  starting_lba;
    uint32_t  number_of_sectors;
} partition_t;

typedef struct {
    uint8_t              bootstrap_code[440];
    uint32_t             disk_signiture;
    uint16_t             copy_protected;
    partition_t          partition_table[4];
    uint16_t             boot_signature;
} mbr_t;

#pragma pack()

void print_partition_geom(partition_t part){
    
    uint16_t s;
    uint16_t c;
    fprintf(stdout, " - start chs:\n");
    s = part.start_chs.cs & 0x3F; 
    c = (part.start_chs.cs & 0xFFC0) >> 6; 
    fprintf(stdout, "   - c:%u h:%u s:%u",
            c,
            part.start_chs.h,
            s);
    if(part.start_chs.h == 254 && s == 63 && c == 1023){
        fprintf(stdout, " (size > 8GB)\n");
    }else{
        fprintf(stdout, "\n");
    }

    fprintf(stdout, " - ending chs:\n");
    s = part.end_chs.cs & 0x3F;
    c = (part.end_chs.cs & 0xFFC0) >> 6;
    fprintf(stdout, "   - c:%u h:%u s:%u",
            c,
            part.end_chs.h,
            s);
    if(part.end_chs.h == 254 && s == 63 && c == 1023){
        fprintf(stdout, " (size > 8GB)\n");
    }else{
        fprintf(stdout, "\n");
    }
    

    fprintf(stdout, " - starting lba: %u\n", part.starting_lba);
    fprintf(stdout, " - ending lba: %u\n", part.starting_lba + part.number_of_sectors - 1);
    fprintf(stdout, " - sectors: %u \n", part.number_of_sectors);
}

void print_partition_status(uint8_t status){
    fprintf(stdout, " - status: ");
    if (status == MBR_STATUS_BOOTABLE) {
        fprintf(stdout, "bootable\n");
        return;
    }
    if (status == MBR_STATUS_NON_BOOTABLE) {
        fprintf(stdout, "non-bootable\n");
        return;
    }
    if (status == MBR_STATUS_BOOTABLE_LBA) {
        fprintf(stdout, "bootable lba\n");
        return;
    }
    if (status == MBR_STATUS_NON_BOOTABLE_LBA) {
        fprintf(stdout, "non-bootable lba\n");
        return;
    }
    fprintf(stdout, "invalid partition status\n");
}

void print_partition_type(uint8_t type){
    fprintf(stdout, " - type: ");
    switch (type) {
        case MBR_TYPE_UNUSED:
            fprintf(stdout, "unused\n");
            break;
        case MBR_TYPE_NTFS:
            fprintf(stdout, "ntfs\n");
            break;
        case MBR_TYPE_LINUX_SWAP:
            fprintf(stdout, "linux-swap\n");
            break;
        case MBR_TYPE_LINUX:
            fprintf(stdout, "linux\n");
            break;
        case MBR_TYPE_EXTENDED_DOS:
            fprintf(stdout, "extended-doc\n");
            break;
        case MBR_TYPE_EXTENDED_WINDOWS:
            fprintf(stdout, "extended-windows\n");
            break;
        default:
            fprintf(stdout, "unknown\n");
            break;
    }
}

int main(int argc, char** argv){
    if(argc != 2){
        fprintf(stderr,"Usage: %s <file name>\n",argv[0]);
        return EXIT_FAILURE;
    }

    FILE *mbr_fd = fopen(argv[1], "r");
    if(mbr_fd == NULL){
        fprintf(stderr, "Error opening mbr file\n");
        return EXIT_FAILURE;
    }

    mbr_t mbr;
    if(fread((void *)&mbr, 1, sizeof(mbr_t), mbr_fd) != sizeof(mbr_t)){
        fprintf(stderr,"Error reading mbr\n");
        return EXIT_FAILURE;
    }

    if(mbr.boot_signature != MBR_BOOT_SIGNATURE){
        fprintf(stderr,"No valid bootsector found\n");
        return EXIT_FAILURE;
    }

    for(int i = 0; i < 4; ++i){
        if(mbr.partition_table[i].type == MBR_TYPE_UNUSED){
            continue;
        }
        fprintf(stdout,"Partition %d\n", i);
        print_partition_status(mbr.partition_table[i].status);
        print_partition_type(mbr.partition_table[i].type);
        print_partition_geom(mbr.partition_table[i]);
    }

    return EXIT_SUCCESS;
}
