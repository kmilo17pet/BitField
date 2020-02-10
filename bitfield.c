/*!
 * *******************************************************************************
 * @file bitfield.c
 * @author J. Camilo Gomez C.
 * @version 1.0.4
 * @date Feb 5 , 2020
 * @brief A portable bit-field implementation.
 *********************************************************************************/
#include "bitfield.h"

#define LONG_BIT                (size_t) (sizeof(uint32_t) * 8 )
#define BITMASK(b)              (1UL << ((b) % LONG_BIT))
#define BITSLOT(b)              ((b) / LONG_BIT) /*((b) >> 6 )*/
#define BITGET(a, b)            (((a)->field[BITSLOT(b)] >> ((b) % LONG_BIT))  & 1UL)
#define BITSET(a, b)            ((a)->field[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b)          ((a)->field[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b)           ((a)->field[BITSLOT(b)] & BITMASK(b))
#define BITTOGGLE(a, b)         ((a)->field[BITSLOT(b)] ^= BITMASK(b))
#define BITNSLOTS(nb)           ((nb + LONG_BIT - 1) / LONG_BIT)
#define BITOFFSET(index)        ((index) & 31)
#define BITMASKMERGE(a,b,abits) (b ^ ((a ^ b) & abits))
#define BITMASK32(nbits)        ((nbits)? ~(uint32_t)0 >> (sizeof(uint32_t)*8-(nbits)) : (uint32_t)0)

static uint32_t bitfield_read_uint32(const bitfield_t *instance, size_t index );
static void bitfield_write_uint32( bitfield_t *instance, size_t index, uint32_t value );

/*============================================================================*/
/**
 * @brief Setup a bitfield instance.
 * @note Example :
 * 	uint8_t bfarea[ BITFIELD_SIZE(96)  ] = {0};
 *	bitfield_setup( &bf, bfarea, sizeof(bfarea));
 * @param instance A pointer to the bitfield instance.
 * @param area A pointer to the memory block to hold the bitfied. Should be
 *        an uint8_t array of size BITFIELD_SIZE(n), where n, is the number of
 *        bits inside the bitfield.
 * @param size The number of bytes in @area.
 * @return none
 */
void bitfield_setup( bitfield_t *instance, void *area, size_t area_size ){
    instance->field = area;
    instance->size = (area_size*8);
    instance->nSlots = area_size/sizeof(uint32_t);
}
/*============================================================================*/
/**
 * @brief Retrieve the state of a bit in a bitfield
 * @param instance A pointer to the bitfield instance.
 * @param index The bit-index
 * @return The value of the bit at @index.
 */
uint8_t bitfield_read_bit( const bitfield_t *instance, size_t index){
    return (BITGET(instance, index))? 1u : 0u;
}
/*============================================================================*/
/**
 * @brief Sets one bit in a bitfield
 * @param instance A pointer to the bitfield instance.
 * @param index The bit-index
 * @return none
 */
void bitfield_set_bit( bitfield_t *instance, size_t index ){
    BITSET( instance, index );
}
/*============================================================================*/
/**
 * @brief Clears one bit in a bitfield
 * @param instance A pointer to the bitfield instance.
 * @param index The bit-index
 * @return none
 */
void bitfield_clear_bit( bitfield_t *instance, size_t index ){
    BITCLEAR( instance, index );
}
/*============================================================================*/
/**
 * @brief Toggles (i.e. reverses the state of) a bit in a bitfield
 * @param instance A pointer to the bitfield instance.
 * @param index The bit-index
 * @return none
 */
void bitfield_toggle_bit( bitfield_t *instance, size_t index ){
    BITTOGGLE( instance, index );
}
/*============================================================================*/
/**
 * @brief Writes one bit in a bitfield
 * @param instance A pointer to the bitfield instance.
 * @param index The bit-index
 * @return none
 */
void bitfield_write_bit( bitfield_t *instance, size_t index, uint8_t value ){
    if( 0u != value ){
        BITSET( instance, index );
    }
    else{
        BITCLEAR( instance, index );
    }
}
/*============================================================================*/
/**
 * @brief Reads an unsigned 32-bit value from the bitfield
 * @param instance A pointer to the bitfield instance.
 * @param index The bit-index taken as offset.
 * @return The value of the bitfield from the desired index
 */
static uint32_t bitfield_read_uint32(const bitfield_t *instance, size_t index ){
    size_t slot;
    uint8_t of, bits_taken;
    uint32_t result;

    slot = BITSLOT(index);
    of = BITOFFSET( index );
    result = instance->field[ slot ] >> of;
    bits_taken  = 32 - of;
    
    if( of > 0 && index + bits_taken < instance->size ){
        result |= instance->field[ slot+1 ] << (uint32_t)bits_taken ;
    }
    return result;
}
/**
 * @brief Writes an unsigned 32-bit value to the bitfield
 * @param instance A pointer to the bitfield instance.
 * @param index The bit-index taken as offset.
 * @return none
 */
/*============================================================================*/
static void bitfield_write_uint32( bitfield_t *instance, size_t index, uint32_t value ){
    uint8_t of;
    uint32_t mask;
    size_t slot;
    slot = BITSLOT( index );
    of = BITOFFSET( index );
    if( 0 == of ){
        instance->field[ slot ] = value;
    }
    else{
        mask = BITMASK32(of);
        instance->field[ slot   ] = ( value<<of ) | ( instance->field[slot] & mask )    ;
        if( slot+1 < instance->nSlots ){
            instance->field[ slot+1 ] = ( value >> (32-of) ) | ( instance->field[ slot+1 ] & ( ~mask ) );  
        }
     }
}
/*============================================================================*/
/**
 * @brief Reads an unsigned n-bit value from the bitfield
 * @param instance A pointer to the bitfield instance.
 * @param index The bit-index taken as offset.
 * @param xbits The number of bits to read. ( max allowed : 32 bits )
 * @return The value from the bitfield from the desired index
 */
uint32_t bitfield_read_uintn( bitfield_t *instance, size_t index, size_t xbits ){
    uint32_t value = 0ul;
    if( xbits <= 32 ){
        if( 1 == xbits ){
            value = (uint32_t)bitfield_read_bit( instance, index );
        }
        else if( 32 == xbits ){
            value = bitfield_read_uint32( instance, index );
        }
        else{
            value = bitfield_read_uint32( instance, index );
            value &= ( (~((uint32_t)0)) >> (32-xbits) ); /*safe mask*/   
        }        
    }
    return value;
}
/**
 * @brief Writes an unsigned n-bit value from the bitfield
 * @param instance A pointer to the bitfield instance.
 * @param index The bit-index taken as offset.
 * @param xbits The number of bits to write. ( max allowed : 32 bits )
 * @return none
 */
/*============================================================================*/
void bitfield_write_uintn( bitfield_t *instance, size_t index, uint32_t value, size_t xbits ){
    uint32_t w, mask;
    if( xbits <= 32 ){
        if( 1 == xbits ){
            bitfield_write_bit( instance, index, (uint8_t)value );
        }
        else if( 32 == xbits ){
            value = bitfield_read_uint32( instance, index );
        }
        else{
            w = bitfield_read_uint32( instance, index );
            value &= ( (~((uint32_t)0)) >> (32-xbits) ); /*safe mask*/
            mask = ( ~((uint32_t)0) ) << xbits;
            bitfield_write_uint32( instance, index, BITMASKMERGE(w, value, mask));         
        }        
    }
}
/*============================================================================*/
/**
 * @brief Reads an 32-bit float value from the bitfield
 * @param instance A pointer to the bitfield instance.
 * @param index The bit-index taken as offset.
 * @return The float-value from the bitfield at the desired index
 */
float bitfield_read_float(const bitfield_t *instance, size_t index ){
    float fval;
    uint32_t rval;
    rval = bitfield_read_uint32( instance, index );
    memcpy( &fval, &rval, sizeof(float) );
    return fval;
}
/*============================================================================*/
/**
 * @brief Writes an 32-bit float value to the bitfield
 * @param instance A pointer to the bitfield instance.
 * @param index The bit-index taken as offset.
 * @return none
 */
void bitfield_write_float( bitfield_t *instance, size_t index, float value ){
    uint32_t fval;
    (void)memcpy( &fval, &value, sizeof(float) );
    bitfield_write_uint32( instance, index, fval );
}
/*============================================================================*/
void* bitfield_dump( bitfield_t *instance, void* dst, size_t n ){
    void *RetValue = NULL; 
    if( n <= (instance->size/8)  ){
        RetValue = memcpy( dst, instance->field, n );
    }
    RetValue;
}
/*============================================================================*/