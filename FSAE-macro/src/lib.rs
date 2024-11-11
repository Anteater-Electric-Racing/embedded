use proc_macro::TokenStream;
use quote::quote;
use syn::{parse_macro_input, Data, DeriveInput, Fields, Type};

/// Derive macro to automatically generate a `SIZE` constant for message frame structs.
#[proc_macro_derive(MessageSize)]
pub fn derive_message_size(input: TokenStream) -> TokenStream {
    let input = parse_macro_input!(input as DeriveInput);
    let struct_name = input.ident;

    // Calculate the total size of all fields.
    let size = match input.data {
        Data::Struct(data_struct) => {
            calculate_size(&data_struct.fields)
        }
        _ => panic!("#[derive(MessageSize)] can only be used on structs"),
    };

    let expanded = quote! {
        impl #struct_name {
            pub const SIZE: usize = #size;
        }
    };

    TokenStream::from(expanded)
}

fn calculate_size(fields: &Fields) -> usize {
    fields.iter().map(field_size).sum()
}

fn field_size(field: &syn::Field) -> usize {
    let var_name = match &field.ty {
        Type::Path(type_path) => {
            let type_ident = type_path.path.segments.last().unwrap().ident.to_string();
            match type_ident.as_str() {
                "DateTime" => 0,
                "u8" => 1,
                "i8" => 1,
                "u16" => 2,
                "i16" => 2,
                "u32" => 4,
                "i32" => 4,
                "u64" => 8,
                "i64" => 8,
                _ => panic!("Unsupported type {} for MessageSize derive macro", type_ident),
            }
        }
        _ => panic!("Unsupported type for MessageSize derive macro"),
    };
    var_name
}
