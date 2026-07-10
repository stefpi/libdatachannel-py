from libdatachannel import Message, RrHandler, RrStats, make_message_from_data


def test_rrhandler_initial_stats_is_none():
    handler = RrHandler()

    assert handler.get_stats() is None


def test_rrhandler_reads_first_report_block_from_compound_rtcp():
    app = bytes([0x80, 0xCC, 0x00, 0x01, 0xDE, 0xAD, 0xBE, 0xEF])
    rr = bytes(
        [
            0x81,
            0xC9,
            0x00,
            0x07,
            0x01,
            0x02,
            0x03,
            0x04,
            0x11,
            0x22,
            0x33,
            0x44,
            0x40,
            0x00,
            0x00,
            0x05,
            0x00,
            0x02,
            0x12,
            0x34,
            0x00,
            0x00,
            0x00,
            0x0A,
            0x12,
            0x34,
            0x56,
            0x78,
            0x00,
            0x01,
            0x00,
            0x00,
        ]
    )
    data = app + rr
    message = make_message_from_data(data, type=Message.Type.Control)
    handler = RrHandler()

    handler.incoming([message], lambda _: None)

    stats = handler.get_stats()
    assert isinstance(stats, RrStats)
    assert stats.ssrc == 0x11223344
    assert stats.fraction_lost == 0x40
    assert stats.packets_lost == 5
    assert stats.highest_seq_no == 0x00021234
    assert stats.jitter == 10
    assert stats.lsr == 0x12345678
    assert stats.dlsr == 0x00010000
    assert message.to_bytes() == data
