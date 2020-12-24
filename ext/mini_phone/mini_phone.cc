#include "mini_phone.h"
#include "phonenumbers/phonenumberutil.h"

using namespace ::i18n::phonenumbers;

static VALUE rb_mMiniPhone;
static VALUE rb_cPhoneNumber;

static VALUE is_phone_number_valid(VALUE self, VALUE str, VALUE cc) {
  PhoneNumber parsed_number;
  PhoneNumberUtil* phone_util = PhoneNumberUtil::GetInstance();

  std::string phone_number(RSTRING_PTR(str), RSTRING_LEN(str));
  std::string country_code(RSTRING_PTR(cc), RSTRING_LEN(cc));

  auto result = phone_util->ParseAndKeepRawInput(phone_number, country_code, &parsed_number);

  if (result == PhoneNumberUtil::NO_PARSING_ERROR && phone_util->IsValidNumber(parsed_number)) {
    return Qtrue;
  } else {
    return Qfalse;
  }
}

static VALUE rb_is_phone_number_valid(VALUE self, VALUE str) {
  VALUE def_cc = rb_iv_get(rb_mMiniPhone, "@default_country_code");

  return is_phone_number_valid(self, str, def_cc);
}

static VALUE rb_is_phone_number_valid_for_country(VALUE self, VALUE str, VALUE cc) {
  return is_phone_number_valid(self, str, cc);
}

static VALUE rb_is_phone_number_invalid(VALUE self, VALUE str) {
  return rb_is_phone_number_valid(self, str) == Qtrue ? Qfalse : Qtrue;
}

static VALUE rb_is_phone_number_invalid_for_country(VALUE self, VALUE str, VALUE cc) {
  return is_phone_number_valid(self, str, cc) == Qtrue ? Qfalse : Qtrue;
}

static VALUE rb_is_phone_number_possible(VALUE self, VALUE str) {
  PhoneNumber parsed_number;
  PhoneNumberUtil* phone_util = PhoneNumberUtil::GetInstance();

  VALUE def_cc = rb_iv_get(rb_mMiniPhone, "@default_country_code");
  std::string phone_number(RSTRING_PTR(str), RSTRING_LEN(str));
  std::string country_code(RSTRING_PTR(def_cc), RSTRING_LEN(def_cc));

  auto result = phone_util->Parse(phone_number, country_code, &parsed_number);

  if (result == PhoneNumberUtil::NO_PARSING_ERROR && phone_util->IsPossibleNumber(parsed_number)) {
    return Qtrue;
  } else {
    return Qfalse;
  }
}

static VALUE rb_is_phone_number_impossible(VALUE self, VALUE str) {
  return rb_is_phone_number_possible(self, str) == Qtrue ? Qfalse : Qtrue;
}

static VALUE rb_set_default_country_code(VALUE self, VALUE str_code) {
  return rb_iv_set(self, "@default_country_code", str_code);
}

static VALUE rb_get_default_country_code(VALUE self) {
  return rb_iv_get(self, "@default_country_code");
}

struct PhoneNumberInfo {
  PhoneNumber phone_number;
  std::string raw_phone_number;
  std::string raw_country_code;
};

void rb_phone_number_dealloc(PhoneNumberInfo* phone_number_info) {
  delete phone_number_info;
}

static VALUE rb_phone_number_alloc(VALUE self) {
  PhoneNumberInfo* phone_number_info = new PhoneNumberInfo();

	/* wrap */
	return Data_Wrap_Struct(self, NULL, &rb_phone_number_dealloc, phone_number_info);
}

static VALUE rb_phone_number_initialize(int argc, VALUE* argv, VALUE self) {
  VALUE str;
  VALUE def_cc;

  rb_scan_args(argc, argv, "11", &str, &def_cc);

  if (NIL_P(def_cc)) {
    def_cc = rb_iv_get(rb_mMiniPhone, "@default_country_code");
  }

  Check_Type(str, T_STRING);

  rb_iv_set(self, "@input", str);

  PhoneNumberInfo* phone_number_info;
  PhoneNumber parsed_number;

	Data_Get_Struct(self, PhoneNumberInfo, phone_number_info);

  PhoneNumberUtil* phone_util = PhoneNumberUtil::GetInstance();

  std::string phone_number(RSTRING_PTR(str), RSTRING_LEN(str));
  std::string country_code(RSTRING_PTR(def_cc), RSTRING_LEN(def_cc));

  auto result = phone_util->Parse(phone_number, country_code, &parsed_number);

  if (result != PhoneNumberUtil::NO_PARSING_ERROR) {
    rb_raise(rb_eArgError, "Could not parse phone number %s", RSTRING_PTR(str));
  } else {
    phone_number_info->phone_number = parsed_number;
  }

	return self;
}

static VALUE rb_phone_number_format(VALUE self, PhoneNumberUtil::PhoneNumberFormat fmt) {
  std::string formatted_number;
  PhoneNumberInfo* phone_number_info;
	Data_Get_Struct(self, PhoneNumberInfo, phone_number_info);

  PhoneNumberUtil* phone_util = PhoneNumberUtil::GetInstance();
  PhoneNumber parsed_number = phone_number_info->phone_number;
  phone_util->Format(parsed_number, fmt, &formatted_number);

  return rb_str_new(formatted_number.c_str(), formatted_number.size());
}

static VALUE rb_phone_number_e164(VALUE self) {
  if (rb_ivar_defined(self, rb_str_new_cstr("@e164"))) {
    return rb_iv_get(self, "@e164");
  }

  return rb_iv_set(self, "@e164", rb_phone_number_format(self, PhoneNumberUtil::PhoneNumberFormat::E164));
}

static VALUE rb_phone_number_national(VALUE self) {
  if (rb_ivar_defined(self, rb_str_new_cstr("@national"))) {
    return rb_iv_get(self, "@national");
  }

  return rb_iv_set(self, "@national", rb_phone_number_format(self, PhoneNumberUtil::PhoneNumberFormat::NATIONAL));
}

static VALUE rb_phone_number_international(VALUE self) {
  if (rb_ivar_defined(self, rb_str_new_cstr("@international"))) {
    return rb_iv_get(self, "@international");
  }

  return rb_iv_set(self, "@international", rb_phone_number_format(self, PhoneNumberUtil::PhoneNumberFormat::INTERNATIONAL));
}

static VALUE rb_phone_number_rfc3966(VALUE self) {
  if (rb_ivar_defined(self, rb_str_new_cstr("@rfc3966"))) {
    return rb_iv_get(self, "@rfc3966");
  }
  return rb_iv_set(self, "@rfc3966", rb_phone_number_format(self, PhoneNumberUtil::PhoneNumberFormat::RFC3966));
}

static VALUE rb_phone_number_valid_eh(VALUE self) {
  std::string formatted_number;
  PhoneNumberInfo* phone_number_info;
	Data_Get_Struct(self, PhoneNumberInfo, phone_number_info);

  PhoneNumberUtil* phone_util = PhoneNumberUtil::GetInstance();

  if (phone_util->IsValidNumber(phone_number_info->phone_number)) {
    return Qtrue;
  } else {
    return Qfalse;
  }
}

static VALUE rb_phone_number_invalid_eh(VALUE self) {
  return rb_phone_number_valid_eh(self) == Qtrue ? Qfalse : Qtrue;
}

static VALUE rb_phone_number_region_code(VALUE self) {
  if (rb_ivar_defined(self, rb_str_new_cstr("@region_code"))) {
    return rb_iv_get(self, "@region_code");
  }

  PhoneNumberInfo* phone_number_info;
  std::string code;
	Data_Get_Struct(self, PhoneNumberInfo, phone_number_info);
  PhoneNumberUtil* phone_util = PhoneNumberUtil::GetInstance();

  phone_util->GetRegionCodeForCountryCode(phone_number_info->phone_number.country_code(), &code);

  VALUE result = rb_str_new(code.c_str(), code.size());

  return rb_iv_set(self, "@region_code", result);
}

static VALUE rb_phone_number_country_code(VALUE self) {
  if (rb_ivar_defined(self, rb_str_new_cstr("@country_code"))) {
    return rb_iv_get(self, "@country_code");
  }

  PhoneNumberInfo* phone_number_info;
	Data_Get_Struct(self, PhoneNumberInfo, phone_number_info);
  PhoneNumberUtil* phone_util = PhoneNumberUtil::GetInstance();

  int code = phone_number_info->phone_number.country_code();

  VALUE result = INT2NUM(code);

  return rb_iv_set(self, "@country_code", result);
}

static VALUE rb_phone_number_eql_eh(VALUE self, VALUE other) {
  if (!rb_obj_is_instance_of(other, rb_cPhoneNumber)) {
    return Qfalse;
  }

  PhoneNumberUtil* phone_util = PhoneNumberUtil::GetInstance();

  PhoneNumberInfo* self_phone_number_info;
	Data_Get_Struct(self, PhoneNumberInfo, self_phone_number_info);

  PhoneNumberInfo* other_phone_number_info;
	Data_Get_Struct(other, PhoneNumberInfo, other_phone_number_info);

  if (phone_util->IsNumberMatch(other_phone_number_info->phone_number, self_phone_number_info->phone_number)) {
    return Qtrue;
  } else {
    return Qfalse;
  }

  // int code = phone_number_info->phone_number.country_code();

  // return INT2NUM(code);
}  

static VALUE rb_phone_number_type(VALUE self) {
  if (rb_ivar_defined(self, rb_str_new_cstr("@type"))) {
    return rb_iv_get(self, "@type");
  }

  PhoneNumberInfo* phone_number_info;
	Data_Get_Struct(self, PhoneNumberInfo, phone_number_info);
  PhoneNumberUtil* phone_util = PhoneNumberUtil::GetInstance();

  VALUE result;

  // @see https://github.com/google/libphonenumber/blob/4e9954edea7cf263532c5dd3861a801104c3f012/cpp/src/phonenumbers/phonenumberutil.h#L91
  switch (phone_util->GetNumberType(phone_number_info->phone_number)) {
    case PhoneNumberUtil::PREMIUM_RATE:
      result = rb_intern("premium_rate");
      break;
    case PhoneNumberUtil::TOLL_FREE:
      result = rb_intern("toll_free");
      break;
    case PhoneNumberUtil::MOBILE:
      result = rb_intern("mobile");
      break;
    case PhoneNumberUtil::FIXED_LINE:
      result = rb_intern("fixed_line");
      break;
    case PhoneNumberUtil::FIXED_LINE_OR_MOBILE:
      result = rb_intern("fixed_line_or_mobile");
      break;
    case PhoneNumberUtil::SHARED_COST:
      result = rb_intern("shared_cost");
      break;
    case PhoneNumberUtil::VOIP:
      result = rb_intern("voip");
      break;
    case PhoneNumberUtil::PERSONAL_NUMBER:
      result = rb_intern("personal_number");
      break;
    case PhoneNumberUtil::PAGER:
      result = rb_intern("pager");
      break;
    case PhoneNumberUtil::UAN:
      result = rb_intern("uan");
      break;
    case PhoneNumberUtil::VOICEMAIL:
      result = rb_intern("voicemail");
      break;
    default:
      result = rb_intern("unknown");
      break;
  }

  return rb_iv_set(self, "@type", ID2SYM(result));
}

extern "C" 
void Init_mini_phone(void)
{
  rb_mMiniPhone = rb_define_module("MiniPhone");

  // Unknown
  rb_iv_set(rb_mMiniPhone, "@default_country_code", rb_str_new("ZZ", 2));

  rb_define_module_function(rb_mMiniPhone, "valid?", rb_is_phone_number_valid, 1);
  rb_define_module_function(rb_mMiniPhone, "valid_for_country?", rb_is_phone_number_valid_for_country, 2);
  rb_define_module_function(rb_mMiniPhone, "invalid?", rb_is_phone_number_invalid, 1);
  rb_define_module_function(rb_mMiniPhone, "invalid_for_country?", rb_is_phone_number_invalid_for_country, 2);
  rb_define_module_function(rb_mMiniPhone, "possible?", rb_is_phone_number_valid, 1);
  rb_define_module_function(rb_mMiniPhone, "impossible?", rb_is_phone_number_invalid, 1);
  rb_define_module_function(rb_mMiniPhone, "default_country_code=", rb_set_default_country_code, 1);
  rb_define_module_function(rb_mMiniPhone, "default_country_code", rb_get_default_country_code, 0);


  rb_cPhoneNumber = rb_define_class_under(rb_mMiniPhone, "PhoneNumber", rb_cObject);

  rb_define_singleton_method(rb_cPhoneNumber, "parse", rb_class_new_instance, -1);
  rb_define_alloc_func(rb_cPhoneNumber, rb_phone_number_alloc);
	rb_define_method(rb_cPhoneNumber, "initialize", rb_phone_number_initialize, -1);
	rb_define_method(rb_cPhoneNumber, "valid?", rb_phone_number_valid_eh, 0);
	rb_define_method(rb_cPhoneNumber, "invalid?", rb_phone_number_invalid_eh, 0);
	rb_define_method(rb_cPhoneNumber, "e164", rb_phone_number_e164, 0);
	rb_define_method(rb_cPhoneNumber, "national", rb_phone_number_national, 0);
	rb_define_method(rb_cPhoneNumber, "international", rb_phone_number_international, 0);
	rb_define_method(rb_cPhoneNumber, "rfc3966", rb_phone_number_rfc3966, 0);
	rb_define_method(rb_cPhoneNumber, "region_code", rb_phone_number_region_code, 0);
	rb_define_method(rb_cPhoneNumber, "country_code", rb_phone_number_country_code, 0);
	rb_define_method(rb_cPhoneNumber, "type", rb_phone_number_type, 0);
	rb_define_method(rb_cPhoneNumber, "eql?", rb_phone_number_eql_eh, 1);
}
