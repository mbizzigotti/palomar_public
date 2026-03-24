#include "../core.h"

struct cJSON;

/**!
 * @brief Loads transformation matrix from JSON object.
 * @param transform JSON object to load from.
 * @return Matrix containing the loaded transform
 * 
 * @note If transform is null, or transform is invalid,
 *       then the identity matrix is returned.
*/
mat4 load_transform(cJSON *transform);
