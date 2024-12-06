CREATE TABLE pollution (
    id TEXT PRIMARY KEY,
    sampling_point TEXT,
    sampling_point_notation TEXT,
    sampling_point_label TEXT,
    sample_date_time TEXT,
    determinand_label TEXT,
    determinand_definition TEXT,
    determinand_notation TEXT,
    result_qualifier TEXT,
    result REAL,
    coded_result_interpretation TEXT,
    determinand_unit_label TEXT,
    sampled_material_type TEXT,
    is_compliance_sample BOOLEAN,
    sample_purpose_label TEXT,
    sampling_point_easting INTEGER,
    sampling_point_northing INTEGER
);