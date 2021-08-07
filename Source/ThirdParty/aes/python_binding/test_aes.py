"""
Tests for the AES Python bindings
"""

import re
import copy
from aes import AES


def _test_ecb(test_vals, test_key):
    for pt_str, ct_str in test_vals:
        pt_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', pt_str)])
        pt_copy = copy.copy(pt_bytes)
        ct_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', ct_str)])
        aes_ecb = AES(mode='ecb', key=test_key)
        aes_ecb.encrypt(pt_bytes)
        assert pt_bytes == ct_bytes, "AES ECB mode encryption failure"
        aes_ecb.decrypt(pt_bytes)
        assert pt_bytes == pt_copy, "AES ECB mode decryption failure"


def test_ecb_128():
    """
    Verify that single-block 128-bit ECB en/decryption works.
    Test vectors from here:
        https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-
        Algorithm-Validation-Program/documents/aes/AESAVS.pdf
    Appendix D. VarTxt Known Answer Test Values -- first 10 values from D.1
    """
    aesavs_d1_key = bytearray((0x00,) * 16)
    aesavs_d1_answers = (
        ('80000000000000000000000000000000', '3ad78e726c1ec02b7ebfe92b23d9ec34'),
        ('c0000000000000000000000000000000', 'aae5939c8efdf2f04e60b9fe7117b2c2'),
        ('e0000000000000000000000000000000', 'f031d4d74f5dcbf39daaf8ca3af6e527'),
        ('f0000000000000000000000000000000', '96d9fd5cc4f07441727df0f33e401a36'),
        ('f8000000000000000000000000000000', '30ccdb044646d7e1f3ccea3dca08b8c0'),
        ('fc000000000000000000000000000000', '16ae4ce5042a67ee8e177b7c587ecc82'),
        ('fe000000000000000000000000000000', 'b6da0bb11a23855d9c5cb1b4c6412e0a'),
        ('ff000000000000000000000000000000', 'db4f1aa530967d6732ce4715eb0ee24b'),
        ('ff800000000000000000000000000000', 'a81738252621dd180a34f3455b4baa2f'),
        ('ffc00000000000000000000000000000', '77e2b508db7fd89234caf7939ee5621a')
    )
    _test_ecb(aesavs_d1_answers, aesavs_d1_key)


def test_ecb_256():
    """
    Verify that single-block 256-bit ECB en/decryption works.
    Test vectors from here:
        https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-
        Algorithm-Validation-Program/documents/aes/AESAVS.pdf
    Appendix D. VarTxt Known Answer Test Values -- first 10 values from D.3
    """
    aesavs_d3_key = bytearray((0x00,) * 32)
    aesavs_d3_answers = (
        ('80000000000000000000000000000000', 'ddc6bf790c15760d8d9aeb6f9a75fd4e'),
        ('c0000000000000000000000000000000', '0a6bdc6d4c1e6280301fd8e97ddbe601'),
        ('e0000000000000000000000000000000', '9b80eefb7ebe2d2b16247aa0efc72f5d'),
        ('f0000000000000000000000000000000', '7f2c5ece07a98d8bee13c51177395ff7'),
        ('f8000000000000000000000000000000', '7818d800dcf6f4be1e0e94f403d1e4c2'),
        ('fc000000000000000000000000000000', 'e74cd1c92f0919c35a0324123d6177d3'),
        ('fe000000000000000000000000000000', '8092a4dcf2da7e77e93bdd371dfed82e'),
        ('ff000000000000000000000000000000', '49af6b372135acef10132e548f217b17'),
        ('ff800000000000000000000000000000', '8bcd40f94ebb63b9f7909676e667f1e7'),
        ('ffc00000000000000000000000000000', 'fe1cffb83f45dcfb38b29be438dbd3ab')
    )
    _test_ecb(aesavs_d3_answers, aesavs_d3_key)


def test_cbc_128_f21():
    """
    Verify that 4-block CBC encryption works
    From NIST special publication 800-38A, section F.2.1
    """
    key_str = '2b7e151628aed2a6abf7158809cf4f3c'
    iv_str = '000102030405060708090a0b0c0d0e0f'
    pt_str = (
        '6bc1bee22e409f96e93d7e117393172a'
        'ae2d8a571e03ac9c9eb76fac45af8e51'
        '30c81c46a35ce411e5fbc1191a0a52ef'
        'f69f2445df4f9b17ad2b417be66c3710'
    )
    ct_str = (
        '7649abac8119b246cee98e9b12e9197d'
        '5086cb9b507219ee95db113a917678b2'
        '73bed6b8e3c1743b7116e69e22229516'
        '3ff1caa1681fac09120eca307586e1a7'
    )
    key_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', key_str)])
    iv_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', iv_str)])
    pt_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', pt_str)])
    pt_copy = copy.copy(pt_bytes)
    ct_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', ct_str)])
    aes_cbc = AES(mode='cbc', key=key_bytes, iv=iv_bytes)
    aes_cbc.encrypt(pt_bytes)
    assert pt_bytes == ct_bytes, "AES CBC mode encryption failure"
    aes_cbc.reset()
    aes_cbc.decrypt(pt_bytes)
    assert pt_bytes == pt_copy, "AES CBC mode decryption failure"


def test_cbc_128_f22():
    """
    From NIST special publication 800-38A, section F.2.2
    """
    key_str = '2b7e151628aed2a6abf7158809cf4f3c'
    iv_str = '000102030405060708090a0b0c0d0e0f'
    ct_str = (
        '7649abac8119b246cee98e9b12e9197d'
        '5086cb9b507219ee95db113a917678b2'
        '73bed6b8e3c1743b7116e69e22229516'
        '3ff1caa1681fac09120eca307586e1a7'
    )
    pt_str = (
        '6bc1bee22e409f96e93d7e117393172a'
        'ae2d8a571e03ac9c9eb76fac45af8e51'
        '30c81c46a35ce411e5fbc1191a0a52ef'
        'f69f2445df4f9b17ad2b417be66c3710'
    )
    key_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', key_str)])
    iv_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', iv_str)])
    pt_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', pt_str)])
    ct_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', ct_str)])
    ct_copy = copy.copy(ct_bytes)
    aes_cbc = AES(mode='cbc', key=key_bytes, iv=iv_bytes)
    aes_cbc.decrypt(ct_bytes)
    assert pt_bytes == ct_bytes, "AES CBC mode encryption failure"
    aes_cbc.reset()
    aes_cbc.encrypt(ct_bytes)
    assert ct_bytes == ct_copy, "AES CBC mode decryption failure"


def test_cfb128_aes192_f315():
    """
    From NIST special publication 800-38A, section F.3.15
    """
    key_str = '8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b'
    iv_str = '000102030405060708090a0b0c0d0e0f'
    pt_str = (
        '6bc1bee22e409f96e93d7e117393172a'
        'ae2d8a571e03ac9c9eb76fac45af8e51'
        '30c81c46a35ce411e5fbc1191a0a52ef'
        'f69f2445df4f9b17ad2b417be66c3710'
    )
    ct_str = (
        'cdc80d6fddf18cab34c25909c99a4174'
        '67ce7f7f81173621961a2b70171d3d7a'
        '2e1e8a1dd59b88b1c8e60fed1efac4c9'
        'c05f9f9ca9834fa042ae8fba584b09ff'
    )
    key_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', key_str)])
    iv_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', iv_str)])
    pt_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', pt_str)])
    pt_copy = copy.copy(pt_bytes)
    ct_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', ct_str)])
    aes_cfb = AES(mode='cfb', key=key_bytes, iv=iv_bytes)
    aes_cfb.encrypt(pt_bytes)
    assert pt_bytes == ct_bytes, "AES CFB mode encryption failure"
    aes_cfb.reset()
    aes_cfb.decrypt(pt_bytes)
    assert pt_bytes == pt_copy, "AES CFB mode decryption failure"


def test_ofb_aes256_f46():
    """
    From NIST special publication 800-38A, section F.4.6
    """
    key_str = '603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4'
    iv_str = '000102030405060708090a0b0c0d0e0f'
    ct_str = (
        'dc7e84bfda79164b7ecd8486985d3860'
        '4febdc6740d20b3ac88f6ad82a4fb08d'
        '71ab47a086e86eedf39d1c5bba97c408'
        '0126141d67f37be8538f5a8be740e484'
    )
    pt_str = (
        '6bc1bee22e409f96e93d7e117393172a'
        'ae2d8a571e03ac9c9eb76fac45af8e51'
        '30c81c46a35ce411e5fbc1191a0a52ef'
        'f69f2445df4f9b17ad2b417be66c3710'
    )
    key_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', key_str)])
    iv_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', iv_str)])
    ct_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', ct_str)])
    ct_copy = copy.copy(ct_bytes)
    pt_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', pt_str)])
    aes_ofb = AES(mode='ofb', key=key_bytes, iv=iv_bytes)
    aes_ofb.decrypt(ct_bytes)
    assert ct_bytes == pt_bytes, "AES OFB mode encryption failure"
    aes_ofb.reset()
    aes_ofb.encrypt(ct_bytes)
    assert ct_bytes == ct_copy, "AES OFB mode decryption failure"


def test_ctr_aes192_f53():
    """
    From NIST special publication 800-38A, section F.5.3
    """
    key_str = '8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b'
    counter_str = 'f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff'
    pt_str = (
        '6bc1bee22e409f96e93d7e117393172a'
        'ae2d8a571e03ac9c9eb76fac45af8e51'
        '30c81c46a35ce411e5fbc1191a0a52ef'
        'f69f2445df4f9b17ad2b417be66c3710'
    )
    ct_str = (
        '1abc932417521ca24f2b0459fe7e6e0b'
        '090339ec0aa6faefd5ccc2c6f4ce8e94'
        '1e36b26bd1ebc670d1bd1d665620abf7'
        '4f78a7f6d29809585a97daec58c6b050'
    )
    key_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', key_str)])
    counter_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', counter_str)])
    pt_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', pt_str)])
    pt_copy = copy.copy(pt_bytes)
    ct_bytes = bytearray([int(v, 16) for v in re.findall(r'..?', ct_str)])
    aes_ctr = AES(mode='ctr', key=key_bytes, iv=counter_bytes)
    aes_ctr.encrypt(pt_bytes)
    assert pt_bytes == ct_bytes, "AES CTR mode encryption failure"
    aes_ctr.reset()
    aes_ctr.decrypt(pt_bytes)
    assert pt_bytes == pt_copy, "AES CTR mode decryption failure"

if __name__ == "__main__":
    test_ecb_128()
    test_ecb_256()
    test_cbc_128_f21()
    test_cbc_128_f22()
    test_cfb128_aes192_f315()
    test_ofb_aes256_f46()
    test_ctr_aes192_f53()
    print('All tests pass.')
    