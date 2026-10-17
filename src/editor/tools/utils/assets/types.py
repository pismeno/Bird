import enum

class AssetType(enum.IntEnum):
    UNKNOWN = 0
    TEXT = 1
    IMAGE = 2

    @classmethod
    def _missing_(cls, value):
        # Handle numeric string ("1", "2")
        if isinstance(value, str):
            if value.isdigit():
                val_int = int(value)
                if val_int in cls._value2member_map_:
                    return cls(val_int)

            # Handle uppercase name match ("TEXT", "IMAGE")
            upper_val = value.upper()
            if upper_val in cls._member_map_:
                return cls[upper_val]

        return cls.UNKNOWN